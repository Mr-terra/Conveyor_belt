# 🔄 Architecture du Flux des États

## 📊 Vue d'Ensemble du Système

Le système utilise une architecture à **3 couches** pour gérer les états :

```
┌─────────────────────────────────────────────────────────────┐
│                         MAIN.CPP                             │
│                    (Orchestrateur Principal)                 │
│                                                              │
│  • Appelle monitorHardware() toutes les 10ms                │
│  • Appelle monitorAllStates() toutes les 10ms               │
│  • Gère la logique métier du scheduler                      │
│  • Envoie les commandes utilisateur (start/stop)            │
└─────────────────────────────────────────────────────────────┘
        ↓ Commandes              ↑ Lecture état
        ↓                        ↑
┌─────────────────────────────────────────────────────────────┐
│                      CONVEYOR.CPP                            │
│                  (Module Hardware Physique)                  │
│                                                              │
│  Variable d'État : _currentState (String privé)             │
│  • "STOPPED" - Moteur arrêté (GRBL = 'I')                   │
│  • "RUNNING" - Moteur en mouvement (GRBL = 'R')             │
│  • "ALARM"   - Module verrouillé (GRBL = 'A')               │
│                                                              │
│  Responsabilités :                                           │
│  1. Lire le hardware via _grbl.readStatus()                 │
│  2. Mapper les codes GRBL → états internes                  │
│  3. Écrire dans _currentState                               │
│  4. Exécuter les commandes (start/stop/move)                │
└─────────────────────────────────────────────────────────────┘
        ↑ Observe via pointeur
        ↑
┌─────────────────────────────────────────────────────────────┐
│                    STATE_MANAGER.CPP                         │
│                  (Surveillant Automatique)                   │
│                                                              │
│  Moniteur d'État : StateMonitor                             │
│  • void* stateVariable → Pointe vers _currentState          │
│  • String previousState → Mémorise l'état précédent         │
│                                                              │
│  Responsabilités :                                           │
│  1. Lire *stateVariable (déréférence le pointeur)           │
│  2. Comparer currentState vs previousState                  │
│  3. Si différent → Appliquer les règles (StateRule)         │
│  4. Déclencher les erreurs via Error::triggerError()        │
└─────────────────────────────────────────────────────────────┘
```

---

## 🎬 Séquence Complète d'un Changement d'État

### Phase 1 : Initialisation (setup)

```cpp
void setup() {
    M5.begin();
    
    // 1️⃣ Initialiser le module Conveyor
    conveyor.initialize(stateManager);
}
```

**Dans `conveyor.initialize()` :**

```cpp
void Conveyor::initialize(StateManager& stateManager) {
    // 1. Initialiser le hardware GRBL
    _grbl.Init(&Wire);
    _grbl.setMode("distance");
    
    // 2. Lire l'état initial depuis le hardware
    readHardwareState();
    //   ↓
    //   └─> _currentState = "STOPPED" (par exemple)
    
    // 3. Enregistrer le monitoring automatique
    configureStateMonitoring(stateManager);
    //   ↓
    //   ├─> stateManager.observeState(&_currentState, "Conveyor", ...)
    //   │    ↓
    //   │    └─> Le StateManager stocke :
    //   │         • Pointeur vers _currentState
    //   │         • previousState = "STOPPED"
    //   │         • Callback stateToString()
    //   │
    //   └─> setupStateRules(stateManager)
    //        ↓
    //        └─> Enregistre les règles de transition :
    //             STOPPED → RUNNING = ERROR_INFO
    //             * → ALARM = ERROR_FATAL
    //             etc.
}
```

**Résultat après setup :**

```
StateManager.stateMonitors[0] = {
    stateVariable: 0x3FFC2A40,  // Adresse de conveyor._currentState
    moduleName: "Conveyor",
    previousState: "STOPPED",
    lastChangeTime: 1234,
    isStuckWarningGiven: false,
    stateToString: Conveyor::stateToString
}
```

---

### Phase 2 : Boucle Principale (loop)

```cpp
void loop() {
    M5.update();
    
    // ✨ ÉTAPE 1 : Lire le hardware
    conveyor.monitorHardware();
    
    // ✨ ÉTAPE 2 : Détecter les changements
    stateManager.monitorAllStates();
    
    // Commandes utilisateur
    if (M5.BtnA.wasPressed()) {
        conveyor.start();
    }
    
    // Mouvement continu
    if (conveyor.getState() == CONVEYOR_STATE_RUNNING) {
        conveyor.move(true);
    }
    
    delay(10);
}
```

---

### ÉTAPE 1 : `conveyor.monitorHardware()`

**Responsabilité : Synchroniser l'état interne avec le hardware réel**

```cpp
void Conveyor::monitorHardware() {
    static unsigned long lastCheck = 0;
    
    if (millis() - lastCheck >= 100) {  // Throttling à 100ms
        readHardwareState();
        //   ↓
        //   └─> LIT l'état du module GRBL
        //        ↓
        //        └─> MET À JOUR _currentState si changement détecté
        
        lastCheck = millis();
    }
}
```

**Détail de `readHardwareState()` :**

```cpp
String Conveyor::readHardwareState() {
    // 1. Interroger le module GRBL
    String status = _grbl.readStatus();
    //                     ↑
    //              Retourne "I" ou "R" ou "A"
    
    // 2. Mapper le code GRBL vers un état
    String newState;
    if (status[0] == 'I') {
        newState = CONVEYOR_STATE_STOPPED;  // "STOPPED"
    } else if (status[0] == 'A') {
        newState = CONVEYOR_STATE_ALARM;    // "ALARM"
    } else {
        newState = CONVEYOR_STATE_RUNNING;  // "RUNNING"
    }
    
    // 3. Mettre à jour SI CHANGEMENT
    if (newState != _currentState) {
        Serial.print("🔄 Hardware: ");
        Serial.print(_currentState);  // Ex: "STOPPED"
        Serial.print(" → ");
        Serial.println(newState);     // Ex: "RUNNING"
        
        _currentState = newState;  // ← ÉCRITURE dans la variable observée
        //    ↑
        //    Le StateManager va détecter ce changement
        //    car il a un pointeur vers cette variable
    }
    
    return newState;
}
```

**État du système après ÉTAPE 1 :**

```
Mémoire 0x3FFC2A40 (conveyor._currentState) = "RUNNING"
                                               ↑
                        Le contenu a changé, mais le StateManager
                        ne l'a pas encore détecté
```

---

### ÉTAPE 2 : `stateManager.monitorAllStates()`

**Responsabilité : Détecter les changements et appliquer les règles**

```cpp
void StateManager::monitorAllStates() {
    for (StateMonitor& monitor : stateMonitors) {
        
        // 1️⃣ LIRE l'état actuel via le pointeur
        String* statePtr = static_cast<String*>(monitor.stateVariable);
        //      ↑
        //      = 0x3FFC2A40 (adresse de conveyor._currentState)
        
        String currentState = monitor.stateToString(*statePtr);
        //                                           ↑
        //                    Déréférence le pointeur → lit "RUNNING"
        //                                           ↓
        //                    stateToString("RUNNING") → retourne "RUNNING"
        
        // 2️⃣ COMPARER avec l'état précédent
        if (currentState != monitor.previousState) {
            //  "RUNNING"   !=  "STOPPED"  → TRUE !
            
            // 🎉 CHANGEMENT DÉTECTÉ !
            Serial.print("🔄 State change detected: ");
            Serial.print(monitor.moduleName);       // "Conveyor"
            Serial.print(" from ");
            Serial.print(monitor.previousState);    // "STOPPED"
            Serial.print(" to ");
            Serial.println(currentState);           // "RUNNING"
            
            // 3️⃣ APPLIQUER les règles
            checkStateRules("Conveyor", "STOPPED", "RUNNING");
            //   ↓
            //   └─> Cherche une règle correspondante :
            //        module="Conveyor" + fromState="STOPPED" + toState="RUNNING"
            //        ↓
            //        Règle trouvée :
            //        {
            //            module: "Conveyor",
            //            fromState: "STOPPED",
            //            toState: "RUNNING",
            //            level: ERROR_INFO,
            //            message: "Motor started (detected from hardware)"
            //        }
            //        ↓
            //        Error::triggerError("Motor started...", ERROR_INFO, "Conveyor");
            //        ↓
            //        Serial: "ℹ️ [Conveyor] Motor started (detected from hardware)"
            
            // 4️⃣ MÉMORISER le nouvel état
            monitor.previousState = currentState;  // "RUNNING"
            monitor.lastChangeTime = millis();
            monitor.isStuckWarningGiven = false;
        }
        else {
            // 5️⃣ PAS DE CHANGEMENT : Vérifier si bloqué
            if (millis() - monitor.lastChangeTime > 10000) {  // 10s
                if (currentState != "WAITING" && currentState != "STOPPED") {
                    String msg = monitor.moduleName + " stuck in: " + currentState;
                    Error::triggerError(msg, ERROR_WARNING, monitor.moduleName);
                    monitor.isStuckWarningGiven = true;
                }
            }
        }
    }
}
```

**Résultat après ÉTAPE 2 :**

```
StateManager.stateMonitors[0].previousState = "RUNNING"  // Mis à jour
Console: "ℹ️ [Conveyor] Motor started (detected from hardware)"
```

---

## 🔁 Cycle Complet : Démarrage du Moteur

### Timeline Détaillée

```
T=0ms
------
main.loop()
  ↓
  M5.update()
  ↓
  conveyor.monitorHardware()
    ↓
    readHardwareState()
      ↓
      _grbl.readStatus() → "I"  (Idle)
      ↓
      _currentState = "STOPPED"  (pas de changement)
  ↓
  stateManager.monitorAllStates()
    ↓
    currentState = "STOPPED"
    previousState = "STOPPED"
    ↓
    Pas de changement détecté
  ↓
  delay(10)


T=500ms
-------
main.loop()
  ↓
  M5.update()
  ↓
  M5.BtnA.wasPressed() → TRUE  // Utilisateur appuie sur BtnA
  ↓
  conveyor.start()
    ↓
    _isRunning = true
    ↓
    // Pas de modification de _currentState ici !
    // C'est le hardware qui dicte l'état


T=600ms
-------
main.loop()
  ↓
  conveyor.monitorHardware()
    ↓
    readHardwareState()
      ↓
      _grbl.readStatus() → "R"  (Running) ← Hardware a changé !
      ↓
      newState = "RUNNING"
      ↓
      newState != _currentState  → TRUE
      ↓
      Serial: "🔄 Hardware: STOPPED → RUNNING"
      ↓
      _currentState = "RUNNING"  ← ÉCRITURE !
                      ↑
                      À l'adresse 0x3FFC2A40


T=610ms
-------
main.loop()
  ↓
  stateManager.monitorAllStates()
    ↓
    *statePtr = "RUNNING"  ← LIT à l'adresse 0x3FFC2A40
    ↓
    currentState = "RUNNING"
    previousState = "STOPPED"
    ↓
    currentState != previousState  → TRUE
    ↓
    Serial: "🔄 State change detected: Conveyor from STOPPED to RUNNING"
    ↓
    checkStateRules("Conveyor", "STOPPED", "RUNNING")
      ↓
      Règle trouvée : STOPPED → RUNNING = ERROR_INFO
      ↓
      Error::triggerError("Motor started...", ERROR_INFO, "Conveyor")
      ↓
      Serial: "ℹ️ [Conveyor] Motor started (detected from hardware)"
    ↓
    monitor.previousState = "RUNNING"  ← MÉMORISATION


T=620ms
-------
main.loop()
  ↓
  if (conveyor.getState() == CONVEYOR_STATE_RUNNING) {
      //     ↑ Retourne _currentState = "RUNNING"
      conveyor.move(true);
      ↓
      _grbl.setMotor(1, 0, 0, 1000);  ← Commande de mouvement
  }
```

---

## 📋 Tableau Comparatif des Responsabilités

| Composant | Rôle | Écrit _currentState ? | Lit _currentState ? | Déclenche Erreurs ? |
|-----------|------|----------------------|--------------------|--------------------|
| **main.cpp** | Orchestration générale | ❌ | ✅ (via getState()) | ❌ |
| **conveyor.cpp** | Interface hardware | ✅ (readHardwareState) | ✅ (getState) | ❌ |
| **state_manager.cpp** | Surveillance automatique | ❌ | ✅ (via pointeur) | ✅ (via règles) |

---

## 🎯 Points Clés de l'Architecture

### 1. **Source Unique de Vérité : Le Hardware**

```cpp
// ❌ MAUVAISE APPROCHE (ancienne)
void Conveyor::start() {
    _grbl.setMotor(...);
    _currentState = "RUNNING";  // ← Assume que ça a marché
}

// ✅ BONNE APPROCHE (actuelle)
void Conveyor::start() {
    _grbl.setMotor(...);
    // Pas de modification de _currentState ici !
    // readHardwareState() s'en chargera à la prochaine lecture
}
```

**Avantage** : Si le hardware refuse de démarrer (alarme, erreur), l'état reste "STOPPED" automatiquement.

---

### 2. **Séparation Lecture/Écriture**

```
conveyor.cpp          → ÉCRIT dans _currentState (selon hardware)
state_manager.cpp     → LIT _currentState (via pointeur)
main.cpp              → LIT _currentState (via getState())
```

**Avantage** : Pas de risque de conflit d'écriture.

---

### 3. **Détection par Comparaison**

```cpp
// StateManager garde une COPIE de l'état précédent
monitor.previousState = "STOPPED";

// À chaque cycle, compare avec l'état actuel
if (currentState != monitor.previousState) {
    // Changement détecté !
}
```

**Avantage** : Ne déclenche les règles qu'**une seule fois** par transition.

---

### 4. **Règles Déclaratives**

```cpp
// Définir les règles au setup
stateManager.addStateRule("Conveyor", "STOPPED", "RUNNING", 
                          ERROR_INFO, "Motor started");

// Elles s'appliquent automatiquement dans monitorAllStates()
```

**Avantage** : Centralisation de toute la logique de surveillance.

---

## 🔍 Flux de Données Simplifié

```
┌─────────┐
│  GRBL   │  Hardware physique
│ Module  │  État réel : 'I', 'R', 'A'
└────┬────┘
     │ readStatus()
     ↓
┌─────────────────────────┐
│ Conveyor::readHardwareState() │
│                         │
│ Mapping :               │
│  'I' → "STOPPED"        │
│  'R' → "RUNNING"        │
│  'A' → "ALARM"          │
│                         │
│ ↓ Écrit dans            │
│ _currentState           │
└────┬────────────────────┘
     │ Adresse : 0x3FFC2A40
     ↓
┌─────────────────────────┐
│ StateManager            │
│                         │
│ Lit via pointeur :      │
│ *statePtr = _currentState│
│                         │
│ Compare :               │
│ currentState ≠ previousState│
│                         │
│ ↓ Si différent          │
│ checkStateRules()       │
└────┬────────────────────┘
     ↓
┌─────────────────────────┐
│ Error::triggerError()   │
│                         │
│ Affiche le message      │
│ Applique les actions    │
└─────────────────────────┘
```

---

## 🛠️ Exemple de Debug

### Ajouter des Traces pour Vérifier le Flux

```cpp
// Dans conveyor.cpp
String Conveyor::readHardwareState() {
    String newState = /* ... */;
    
    if (newState != _currentState) {
        Serial.print("[CONVEYOR] Write to address ");
        Serial.print((unsigned long)&_currentState, HEX);
        Serial.print(": ");
        Serial.print(_currentState);
        Serial.print(" → ");
        Serial.println(newState);
        
        _currentState = newState;
    }
    return newState;
}
```

```cpp
// Dans state_manager.cpp
void StateManager::monitorAllStates() {
    for (StateMonitor& monitor : stateMonitors) {
        String* statePtr = static_cast<String*>(monitor.stateVariable);
        
        Serial.print("[STATE_MANAGER] Read from address ");
        Serial.print((unsigned long)statePtr, HEX);
        Serial.print(": ");
        Serial.println(*statePtr);
        
        String currentState = monitor.stateToString(*statePtr);
        // ...
    }
}
```

**Sortie attendue :**

```
[CONVEYOR] Write to address 3FFC2A40: STOPPED → RUNNING
[STATE_MANAGER] Read from address 3FFC2A40: RUNNING
🔄 State change detected: Conveyor from STOPPED to RUNNING
ℹ️ [Conveyor] Motor started (detected from hardware)
```

**Preuve que les deux pointent vers la même adresse !** ✅

---

## 📚 Résumé des Flux

### Flux 1 : Commande Utilisateur → Hardware

```
BtnA pressé
  ↓
main.cpp: conveyor.start()
  ↓
conveyor.cpp: _grbl.setMotor(...)
  ↓
Module GRBL reçoit la commande
  ↓
Hardware démarre physiquement
```

### Flux 2 : Hardware → État Interne

```
Module GRBL en mouvement
  ↓
conveyor.cpp: monitorHardware()
  ↓
readHardwareState(): _grbl.readStatus() → "R"
  ↓
Mapping: "R" → "RUNNING"
  ↓
_currentState = "RUNNING"
```

### Flux 3 : État Interne → Détection → Action

```
_currentState = "RUNNING" (à l'adresse 0x3FFC2A40)
  ↓
state_manager.cpp: monitorAllStates()
  ↓
Lit via pointeur: *statePtr → "RUNNING"
  ↓
Compare: "RUNNING" ≠ "STOPPED" (previousState)
  ↓
checkStateRules("Conveyor", "STOPPED", "RUNNING")
  ↓
Règle trouvée: ERROR_INFO
  ↓
Error::triggerError("Motor started...", ERROR_INFO, "Conveyor")
  ↓
Console: "ℹ️ [Conveyor] Motor started (detected from hardware)"
```

---

## 🎓 Conclusion

**Architecture en 3 couches :**

1. **main.cpp** : Chef d'orchestre qui coordonne
2. **conveyor.cpp** : Expert hardware qui lit et écrit l'état réel
3. **state_manager.cpp** : Surveillant qui observe et réagit

**Principe fondamental :**
> Le hardware est la source de vérité.  
> Le code s'adapte au hardware, pas l'inverse.

**Bénéfices :**
- ✅ Détection automatique des problèmes hardware
- ✅ Pas de désynchronisation possible
- ✅ Code déclaratif et maintenable
- ✅ Centralisation de la logique de surveillance

---