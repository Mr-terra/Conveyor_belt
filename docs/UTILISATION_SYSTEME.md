# Documentation Utilisateur - Système de Conveyor RFID

## Table des Matières
1. [Aperçu du Système](#aperçu-du-système)
2. [Architecture Globale](#architecture-globale)
3. [Mise en Service](#mise-en-service)
4. [Utilisation Quotidienne](#utilisation-quotidienne)
5. [Gestion des Erreurs](#gestion-des-erreurs)
6. [Configuration Avancée](#configuration-avancée)
7. [Dépannage](#dépannage)

---

## Aperçu du Système

Le système de conveyor RFID est conçu pour automatiser le tri de colis en utilisant :
- Un convoyeur motorisé (Conveyor)
- Un lecteur RFID pour identifier les colis
- Un système de tri automatique (Servo Sorter)
- Une intégration avec Dolibarr pour la gestion des stocks
- Un système de monitoring d'état automatique

---

## Architecture Globale

### Composants Principaux

```
┌───────────────────────────────────────────────────────┐
│                 SCHEDULER (Chef d'orchestre)          │
└───────────────┬───────────────┬───────────────┬───────┘
                │               │               │
                ▼               ▼               ▼
┌─────────────────────┐ ┌─────────────────┐ ┌─────────────────┐
│   CHECK CONNECT    │ │  INIT POSITION  │ │   CONVEYOR      │
└─────────────────────┘ └─────────────────┘ └─────────────────┘
                │                               │
                ▼                               ▼
┌───────────────────────────────────────────────────────┐
│                     SORTER CONTROLLER                  │
│  ┌─────────────┐    ┌─────────────┐    ┌───────────┐   │
│  │ RFID READER │    │  ADRESSER   │    │ SERVO     │   │
│  └─────────────┘    └─────────────┘    └───────────┘   │
└───────────────────────────────────────────────────────┘
                │
                ▼
┌───────────────────────────────────────────────────────┐
│                     DOLIBARR API                      │
└───────────────────────────────────────────────────────┘
```

### Flux de Travail Automatique

```
[WAITING] 
       │ (Appui Bouton A)
       ▼
[CHECK_CONNECT] → Vérifie toutes les connexions
       │
       ├─ OK → [INIT_POSITIONS] → Initialise les positions
       │                         │
       │                         ├─ OK → [PRODUCTION] → Mode production
       │                         │                         │
       │                         │                         ├─ Conveyor avance
       │                         │                         ├─ RFID lit les tags
       │                         │                         └─ Servo trie les colis
       │                         │
       └─ KO → [WAITING]        └─ KO → [WAITING]
```

---

## Mise en Service

### Prérequis

1. **Matériel requis** :
   - M5Stack avec module GRBL 13.2
   - Lecteur RFID compatible
   - Servo moteur pour le tri
   - Alimentation stable 12V/24V
   - Connexion réseau WiFi

2. **Logiciel requis** :
   - PlatformIO ou Arduino IDE
   - Bibliothèques M5Stack installées
   - Accès à l'API Dolibarr

### Configuration Initial

#### 1. Configuration WiFi

Modifier dans `src/main.cpp` :

```cpp
// Configuration WiFi (à ajouter si nécessaire)
const char* WIFI_SSID = "votre-reseau-wifi";
const char* WIFI_PASSWORD = "votre-mot-de-passe";
```

#### 2. Configuration Dolibarr

Modifier dans `src/main.cpp` :

```cpp
// Configuration Dolibarr
const String DOLIBARR_URL = "http://votre-serveur-dolibarr.com/api/index.php";
const String DOLIBARR_API_KEY = "votre-cle-api-dolibarr";
```

#### 3. Configuration Matérielle

- **Adresse I2C** : Par défaut 0x70 pour le module GRBL
- **Vitesse convoyeur** : Réglable via `conveyor.setSpeed(1000)` (0-1000)
- **Pas de mouvement** : Réglable via `conveyor.setMovement(1)`

### Procédure de Démarrage

1. **Alimentation** :
   - Brancher l'alimentation 12V/24V
   - Vérifier que tous les voyants sont allumés

2. **Initialisation** :
   - Le système démarre en état `WAITING`
   - Vérifier dans le moniteur série : "System initialized. State: WAITING"

3. **Vérification des connexions** :
   - Appuyer sur **Bouton A** pour lancer la vérification
   - Le système vérifie :
     - Connexion Dolibarr
     - Connexion moteur (I2C)
     - Connexion sorter (RFID + Servo)

4. **Initialisation des positions** :
   - Si les connexions sont OK, passage automatique à l'initialisation
   - Les composants se positionnent en position de départ

5. **Mode Production** :
   - Si l'initialisation est OK, passage automatique en production
   - Le convoyeur commence à avancer
   - Le système est prêt à lire les tags RFID

---

## Utilisation Quotidienne

### Interface Utilisateur (Boutons M5Stack)

| Bouton | Fonction | Description |
|--------|----------|-------------|
| **A**  | Démarrer | Lance le processus automatique (vérification → initialisation → production) |
| **B**  | Arrêter  | Arrête le système et retourne à l'état WAITING |
| **C**  | Info     | Affiche l'état courant du système et du convoyeur |

### Cycle de Production Normal

1. **Préparation** :
   - S'assurer que le convoyeur est vide
   - Vérifier que tous les voyants sont verts
   - Appuyer sur **Bouton C** pour vérifier l'état : "WAITING"

2. **Démarrage** :
   - Appuyer sur **Bouton A** pour démarrer
   - Le système effectue automatiquement :
     - Vérification des connexions (≈5-10s)
     - Initialisation des positions (≈3-5s)
     - Passage en mode production

3. **Production** :
   - Placer les colis sur le convoyeur
   - Le système lit automatiquement les tags RFID
   - Les colis sont triés selon la configuration
   - Le convoyeur avance continuellement

4. **Arrêt** :
   - Appuyer sur **Bouton B** pour arrêter
   - Le convoyeur s'arrête progressivement
   - Le système retourne à l'état WAITING

### Indicateurs Visuels

| État | Indicateur | Description |
|------|------------|-------------|
| WAITING | LED Bleue | Système prêt, en attente de démarrage |
| CHECK_CONNECT | LED Jaune clignotante | Vérification des connexions en cours |
| INIT_POSITIONS | LED Jaune fixe | Initialisation des positions en cours |
| PRODUCTION | LED Verte | Mode production actif |
| ERROR | LED Rouge clignotante | Erreur système (voir moniteur série) |

---

## Gestion des Erreurs

### Erreurs Courantes et Solutions

| Erreur | Cause Probable | Solution |
|--------|----------------|----------|
| "Conveyor not initialized" | Problème de connexion I2C | Vérifier le câblage du module GRBL |
| "RFID not detected" | Lecteur RFID non connecté | Vérifier le câblage RFID |
| "Dolibarr connection FAILED" | Réseau ou API Dolibarr HS | Vérifier WiFi et URL API |
| "Servo not responding" | Servo non alimenté | Vérifier alimentation servo |
| "GRBL Motor not detected" | Adresse I2C incorrecte | Vérifier l'adresse (0x70 par défaut) |

### Procédure de Redémarrage

1. **Arrêt d'urgence** :
   - Appuyer sur **Bouton B** pour arrêter le système
   - Si le système ne répond pas, couper l'alimentation

2. **Diagnostic** :
   - Brancher le moniteur série (115200 bauds)
   - Lire les messages d'erreur
   - Identifier le composant défaillant

3. **Redémarrage** :
   - Corriger le problème identifié
   - Couper et rerallumer l'alimentation
   - Le système redémarre en état WAITING

---

## Configuration Avancée

### Réglage de la Vitesse du Conveyor

```cpp
// Dans setup() après conveyor.initialize():
conveyor.setSpeed(800);  // Valeur entre 0 (arrêt) et 1000 (max)
```

### Réglage du Pas de Mouvement

```cpp
// Dans setup() après conveyor.initialize():
conveyor.setMovement(2);  // Nombre de pas par mouvement
```

### Configuration des Bacs de Tri

Modifier dans `src/sorter/adresser/adresser.cpp` :

```cpp
// Mapping entre UID RFID et bacs de destination
Bin Adresser::resolve(String uid) {
    if (uid == "04:1A:2B:3C:4D:5E:6F") return Bin::BIN_1;
    if (uid == "04:1A:2B:3C:4D:5F:7A") return Bin::BIN_2;
    // Ajouter vos mappings ici
    return Bin::NONE; // Inconnu
}
```

### Activation/Désactivation de Dolibarr

```cpp
// Pour désactiver Dolibarr (mode hors ligne):
// Dans setup(), ne pas configurer l'API Dolibarr:
// checkConnect.setDolibarrAPI(nullptr);
```

---

## Dépannage

### Problèmes de Connexion I2C

1. Vérifier le câblage : SDA → GPIO21, SCL → GPIO22
2. Vérifier l'alimentation 5V du module GRBL
3. Tester avec un scanner I2C :
   ```cpp
   // Ajouter dans setup() pour diagnostic:
   Wire.begin();
   byte error, address;
   int nDevices = 0;
   for(address = 1; address < 127; address++ ) {
       Wire.beginTransmission(address);
       error = Wire.endTransmission();
       if (error == 0) {
           Serial.print("I2C device found at 0x");
           Serial.println(address, HEX);
           nDevices++;
       }
   }
   if (nDevices == 0) Serial.println("No I2C devices found");
   ```

### Problèmes RFID

1. Vérifier que l'antenne RFID est bien connectée
2. Tester avec un tag connu
3. Vérifier la distance (max 5-10cm selon le tag)
4. Essayer un autre tag RFID

### Problèmes Réseau

1. Vérifier la connexion WiFi
2. Tester l'URL Dolibarr dans un navigateur
3. Vérifier que l'API key est valide
4. Tester avec un outil comme Postman

### Moniteur Série

**Configuration** : 115200 bauds

**Commandes utiles** :
- `BtnC` : Affiche l'état courant
- Les messages d'erreur sont détaillés
- Les transitions d'état sont loguées

---

## Maintenance Préventive

### Vérifications Quotidiennes
1. Nettoyer les rouleaux du convoyeur
2. Vérifier le serrage des courroies
3. Tester la lecture RFID avec un tag connu
4. Vérifier le mouvement du servo

### Vérifications Hebdomadaires
1. Lubrifier les roulements
2. Vérifier les connexions électriques
3. Tester le bouton d'arrêt d'urgence
4. Mettre à jour le firmware si nécessaire

### Journal de Maintenance

Tenir un journal avec :
- Date et heure
- Problème rencontré
- Solution appliquée
- Pièces remplacées
- Temps d'arrêt

---

## Glossaire

- **GRBL** : Firmware open-source pour contrôle de moteurs
- **I2C** : Bus de communication pour périphériques
- **RFID** : Identification par radiofréquence
- **UID** : Identifiant unique d'un tag RFID
- **API** : Interface de programmation pour Dolibarr
- **State Manager** : Système de gestion d'états automatique

---

## Support

Pour toute question ou problème :
- Consulter la documentation technique dans `docs/`
- Vérifier les exemples dans `examples/`
- Contacter l'équipe de support : support@conveyor-system.com

---

**Version** : 1.0
**Dernière mise à jour** : 2023-11-15
**Auteur** : Équipe Conveyor RFID
