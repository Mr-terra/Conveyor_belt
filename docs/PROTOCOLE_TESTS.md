# PROTOCOLE DE TESTS - PROJET IOT TAPIS ROULANT

## TABLE DES MATIÈRES

1. [Tests par Module](#tests-par-module)
2. [Tests d'Intégration](#tests-dintégration)
3. [Tests de l'Application Complète](#tests-de-lapplication-complète)
4. [Procédures d'Intervention](#procédures-dintervention)
5. [Génération et Exécution des Tests](#génération-et-exécution-des-tests)

---

## TESTS PAR MODULE

### MODULE 1: ORDONNANCEUR (Scheduler)

#### Objectif

Vérifier la gestion des états du système et les transitions entre les phases.

#### Tests Unitaires

##### TEST-ORD-001: État Initial

- **Préconditions**: Système au démarrage
- **Actions**: Démarrer l'ordonnanceur
- **Résultat attendu**: État = "Attente"
- **Critères de succès**: Variable d'état = "Attente"

##### TEST-ORD-002: Transition Attente → Check Connect

- **Préconditions**: État = "Attente"
- **Actions**: Déclencher la vérification de connexion
- **Résultat attendu**: Passage à l'état "Check Connect"
- **Critères de succès**: État change correctement

##### TEST-ORD-003: Condition Is_Connection = true

- **Préconditions**: Check Connect terminé avec succès
- **Actions**: Vérifier la variable Is_Connection
- **Résultat attendu**: Is_Connection = true, passage à "Init Positions"
- **Critères de succès**: Transition correcte vers Init Positions

##### TEST-ORD-004: Condition Is_Prod_Ok = true

- **Préconditions**: Init Positions terminé avec succès
- **Actions**: Vérifier la variable Is_Prod_Ok
- **Résultat attendu**: Is_Prod_Ok = true, passage à "Production"
- **Critères de succès**: Transition correcte vers Production

##### TEST-ORD-005: Gestion BP_Stop

- **Préconditions**: Système en production
- **Actions**: Activer BP_Stop = true
- **Résultat attendu**: Retour à l'état "Attente"
- **Critères de succès**: Arrêt immédiat et retour à Attente

##### TEST-ORD-006: Gestion Erreur_Critique

- **Préconditions**: Système en production
- **Actions**: Déclencher Erreur_Critique = true
- **Résultat attendu**: Retour à l'état "Attente"
- **Critères de succès**: Arrêt immédiat et retour à Attente

##### TEST-ORD-007: Boucle complète

- **Préconditions**: Système initialisé
- **Actions**: Exécuter un cycle complet (Attente → Check → Init → Production → Attente)
- **Résultat attendu**: Cycle complet sans erreur
- **Critères de succès**: Toutes les transitions s'exécutent correctement

---

### MODULE 2: CHECK CONNECT (Vérification des Connexions)

#### Objectif

Vérifier la connectivité de tous les équipements du système.

#### Tests Unitaires

##### TEST-CHK-001: Test Connexion Dolibarr

- **Préconditions**: Système en état "Attente"
- **Actions**: Tester la connexion à Dolibarr
- **Résultat attendu**: Connexion établie, retour OK
- **Critères de succès**: Statut connexion Dolibarr = OK

##### TEST-CHK-002: Test Connexion Moteur Tapis

- **Préconditions**: Connexion Dolibarr OK
- **Actions**: Tester la connexion au moteur du tapis
- **Résultat attendu**: Connexion établie, retour OK
- **Critères de succès**: Statut connexion moteur = OK

##### TEST-CHK-003: Test Connexion Trieur

- **Préconditions**: Connexion moteur tapis OK
- **Actions**: Tester la connexion au trieur
- **Résultat attendu**: Connexion établie, retour OK
- **Critères de succès**: Statut connexion trieur = OK

##### TEST-CHK-004: Test Connexion Capteur RFID

- **Préconditions**: Connexion trieur OK
- **Actions**: Tester la connexion au capteur RFID
- **Résultat attendu**: Connexion établie, retour OK
- **Critères de succès**: Statut connexion RFID = OK

##### TEST-CHK-005: Validation Is_Connect_OK

- **Préconditions**: Tous les tests de connexion passés
- **Actions**: Vérifier la variable Is_Connect_OK
- **Résultat attendu**: Is_Connect_OK = true
- **Critères de succès**: Variable correctement mise à jour

##### TEST-CHK-006: Échec Connexion Dolibarr

- **Préconditions**: Dolibarr inaccessible
- **Actions**: Tenter la connexion
- **Résultat attendu**: Erreur détectée, Is_Connect_OK = false
- **Critères de succès**: Erreur remontée correctement

##### TEST-CHK-007: Échec Connexion Moteur

- **Préconditions**: Moteur déconnecté
- **Actions**: Tenter la connexion
- **Résultat attendu**: Erreur détectée, Is_Connect_OK = false
- **Critères de succès**: Erreur remontée correctement

##### TEST-CHK-008: Échec Connexion Trieur

- **Préconditions**: Trieur déconnecté
- **Actions**: Tenter la connexion
- **Résultat attendu**: Erreur détectée, Is_Connect_OK = false
- **Critères de succès**: Erreur remontée correctement

##### TEST-CHK-009: Échec Connexion RFID

- **Préconditions**: Capteur RFID déconnecté
- **Actions**: Tenter la connexion
- **Résultat attendu**: Erreur détectée, Is_Connect_OK = false
- **Critères de succès**: Erreur remontée correctement

##### TEST-CHK-010: Séquence complète de vérification

- **Préconditions**: Tous les équipements connectés
- **Actions**: Exécuter la séquence complète
- **Résultat attendu**: Tous les tests passent, Is_Connect_OK = true
- **Critères de succès**: Séquence complète sans erreur

---

### MODULE 3: INIT POSITION (Initialisation des Positions)

#### Objectif

Vérifier l'initialisation correcte des positions des équipements.

#### Tests Unitaires

##### TEST-INI-001: Vérification Is_Connect

- **Préconditions**: Check Connect terminé avec succès
- **Actions**: Vérifier Is_Connect avant initialisation
- **Résultat attendu**: Is_Connect = true
- **Critères de succès**: Condition validée

##### TEST-INI-002: Chargement Paramètres Moteur Tapis

- **Préconditions**: Is_Connect = true
- **Actions**: Charger les paramètres du moteur
- **Résultat attendu**: Paramètres chargés correctement
- **Critères de succès**: Données param chargées OK = true

##### TEST-INI-003: Échec Chargement Paramètres

- **Préconditions**: Paramètres corrompus ou inaccessibles
- **Actions**: Tenter le chargement
- **Résultat attendu**: Erreur levée
- **Critères de succès**: Erreur détectée et signalée

##### TEST-INI-004: Init Position Trieur

- **Préconditions**: Paramètres moteur chargés
- **Actions**: Initialiser la position du trieur
- **Résultat attendu**: Trieur en position initiale
- **Critères de succès**: Position trieur = position_init

##### TEST-INI-005: Validation Is_Prod_Ok

- **Préconditions**: Init position trieur terminée
- **Actions**: Vérifier Is_Prod_Ok
- **Résultat attendu**: Is_Prod_Ok = true
- **Critères de succès**: Variable correctement mise à jour

##### TEST-INI-006: Séquence complète d'initialisation

- **Préconditions**: Connexions OK
- **Actions**: Exécuter toute la séquence d'initialisation
- **Résultat attendu**: Système prêt pour production
- **Critères de succès**: Is_Prod_Ok = true, tous les équipements initialisés

##### TEST-INI-007: Timeout Initialisation Trieur

- **Préconditions**: Trieur bloqué mécaniquement
- **Actions**: Lancer l'initialisation
- **Résultat attendu**: Timeout détecté, erreur levée
- **Critères de succès**: Erreur de timeout remontée

---

### MODULE 4: PRODUCTION (Cycle de Production)

#### Objectif

Vérifier le fonctionnement du cycle de production complet.

#### Tests Unitaires

##### TEST-PRD-001: Vérification Is_Prod

- **Préconditions**: Init terminée avec succès
- **Actions**: Vérifier Is_Prod avant démarrage
- **Résultat attendu**: Is_Prod = true
- **Critères de succès**: Condition validée

##### TEST-PRD-002: Démarrage Tapis Roulant

- **Préconditions**: Is_Prod = true
- **Actions**: Faire rouler le tapis
- **Résultat attendu**: Tapis en mouvement à vitesse configurée
- **Critères de succès**: Vitesse tapis = vitesse_config

##### TEST-PRD-003: Lecture Colis RFID

- **Préconditions**: Tapis en mouvement, colis sur tapis
- **Actions**: Lecture des informations RFID
- **Résultat attendu**: Informations lues correctement
- **Critères de succès**: Info_lu != empty, données valides

##### TEST-PRD-004: Triage Colis

- **Préconditions**: Information lue
- **Actions**: Trier le colis selon les informations
- **Résultat attendu**: Colis trié = true
- **Critères de succès**: Colis dirigé vers la bonne sortie

##### TEST-PRD-005: Bouton d'Arrêt (Non activé)

- **Préconditions**: Production en cours
- **Actions**: Vérifier l'état du bouton d'arrêt
- **Résultat attendu**: Bouton = false, production continue
- **Critères de succès**: Cycle continue normalement

##### TEST-PRD-006: Bouton d'Arrêt (Activé)

- **Préconditions**: Production en cours
- **Actions**: Activer le bouton d'arrêt
- **Résultat attendu**: Is_Prod = false, arrêt du cycle
- **Critères de succès**: Production s'arrête immédiatement

##### TEST-PRD-007: Cycle Complet avec 1 Colis

- **Préconditions**: Système en production
- **Actions**: Traiter 1 colis du début à la fin
- **Résultat attendu**: Colis lu, trié et évacué
- **Critères de succès**: Cycle complet sans erreur

##### TEST-PRD-008: Cycle Complet avec 10 Colis

- **Préconditions**: Système en production
- **Actions**: Traiter 10 colis consécutifs
- **Résultat attendu**: Tous les colis traités correctement
- **Critères de succès**: 10 colis triés sans erreur

##### TEST-PRD-009: Colis sans RFID

- **Préconditions**: Production en cours
- **Actions**: Colis sans tag RFID passe
- **Résultat attendu**: Erreur détectée, colis non trié
- **Critères de succès**: Erreur remontée, colis en défaut

##### TEST-PRD-010: RFID Illisible

- **Préconditions**: Production en cours
- **Actions**: Tag RFID endommagé
- **Résultat attendu**: Erreur de lecture détectée
- **Critères de succès**: Erreur remontée correctement

---

### MODULE 5: GESTION DES ERREURS (Error Management)

#### Objectif

Vérifier la détection et la gestion des erreurs du système.

#### Tests Unitaires

##### TEST-ERR-001: Détection Erreur

- **Préconditions**: Système en fonctionnement
- **Actions**: Simuler une erreur
- **Résultat attendu**: Erreur détectée
- **Critères de succès**: État = "Erreur détectée"

##### TEST-ERR-002: Classification Erreur Non Critique

- **Préconditions**: Erreur détectée
- **Actions**: Classifier l'erreur (non critique)
- **Résultat attendu**: Erreur_critique = false
- **Critères de succès**: Passage à auto-récupération

##### TEST-ERR-003: Auto-Récupération Retry

- **Préconditions**: Erreur non critique
- **Actions**: Tenter un retry
- **Résultat attendu**: Récupération réussie
- **Critères de succès**: Récupération_réussie = true

##### TEST-ERR-004: Auto-Récupération Reset Partiel

- **Préconditions**: Erreur non critique
- **Actions**: Effectuer un reset partiel
- **Résultat attendu**: Module réinitialisé
- **Critères de succès**: Récupération_réussie = true

##### TEST-ERR-005: Auto-Récupération Rechargement Paramètres

- **Préconditions**: Erreur non critique
- **Actions**: Recharger les paramètres
- **Résultat attendu**: Paramètres rechargés
- **Critères de succès**: Récupération_réussie = true

##### TEST-ERR-006: Échec Auto-Récupération

- **Préconditions**: Erreur non critique, 3 tentatives échouées
- **Actions**: Tentatives de récupération
- **Résultat attendu**: Reclassement en erreur critique
- **Critères de succès**: Erreur_critique = true

##### TEST-ERR-007: Retour Module d'Origine

- **Préconditions**: Récupération réussie
- **Actions**: Retourner au module d'origine
- **Résultat attendu**: Reprise du fonctionnement normal
- **Critères de succès**: État = état_précédent

##### TEST-ERR-008: Classification Erreur Critique

- **Préconditions**: Erreur détectée
- **Actions**: Classifier l'erreur (critique)
- **Résultat attendu**: Erreur_critique = true
- **Critères de succès**: Passage à mise en sécurité

##### TEST-ERR-009: Mise en Sécurité - Stop Tapis

- **Préconditions**: Erreur critique
- **Actions**: Arrêter le tapis
- **Résultat attendu**: Tapis arrêté
- **Critères de succès**: Vitesse_tapis = 0

##### TEST-ERR-010: Mise en Sécurité - Stop Trieur

- **Préconditions**: Erreur critique
- **Actions**: Arrêter le trieur
- **Résultat attendu**: Trieur arrêté
- **Critères de succès**: Mouvement_trieur = 0

##### TEST-ERR-011: Mise en Sécurité - Log Erreur

- **Préconditions**: Erreur critique
- **Actions**: Logger l'erreur
- **Résultat attendu**: Erreur enregistrée
- **Critères de succès**: Log créé avec timestamp et détails

##### TEST-ERR-012: Attente Action Opérateur

- **Préconditions**: Mise en sécurité effectuée
- **Actions**: Attendre intervention
- **Résultat attendu**: Système en attente
- **Critères de succès**: État = "Attente_opérateur"

##### TEST-ERR-013: Action Opérateur - Reset

- **Préconditions**: Attente opérateur
- **Actions**: Opérateur effectue un reset
- **Résultat attendu**: Système réinitialisé
- **Critères de succès**: Retour à état initial

##### TEST-ERR-014: Action Opérateur - Diagnostic (Si le temps le permet)

- **Préconditions**: Attente opérateur
- **Actions**: Opérateur lance diagnostic
- **Résultat attendu**: Rapport de diagnostic généré
- **Critères de succès**: Rapport disponible

##### TEST-ERR-015: Action Opérateur - Arrêt

- **Préconditions**: Attente opérateur
- **Actions**: Opérateur arrête le système
- **Résultat attendu**: Système arrêté proprement
- **Critères de succès**: Arrêt complet sans dommage

---

### MODULE 6: TRIER COLIS (Package Sorting)

#### Objectif

Vérifier le fonctionnement du système de tri des colis.

#### Tests Unitaires

##### TEST-TRI-001: Réception Info Lu

- **Préconditions**: Lecture RFID effectuée
- **Actions**: Recevoir les informations du colis
- **Résultat attendu**: Info_lu != empty
- **Critères de succès**: Données reçues correctement

##### TEST-TRI-002: Définir Direction - Sortie A

- **Préconditions**: Info lu disponible
- **Actions**: Analyser les données, définir direction A
- **Résultat attendu**: Direction = A
- **Critères de succès**: Direction correctement définie

##### TEST-TRI-003: Définir Direction - Sortie B

- **Préconditions**: Info lu disponible
- **Actions**: Analyser les données, définir direction B
- **Résultat attendu**: Direction = B
- **Critères de succès**: Direction correctement définie

##### TEST-TRI-004: Définir Direction - Sortie C

- **Préconditions**: Info lu disponible
- **Actions**: Analyser les données, définir direction C
- **Résultat attendu**: Direction = C
- **Critères de succès**: Direction correctement définie

##### TEST-TRI-005: Mouvement Trieur vers A

- **Préconditions**: Direction = A
- **Actions**: Bouger le trieur vers sortie A
- **Résultat attendu**: Trieur en position A
- **Critères de succès**: Position_trieur = A

##### TEST-TRI-006: Mouvement Trieur vers B

- **Préconditions**: Direction = B
- **Actions**: Bouger le trieur vers sortie B
- **Résultat attendu**: Trieur en position B
- **Critères de succès**: Position_trieur = B

##### TEST-TRI-007: Mouvement Trieur vers C

- **Préconditions**: Direction = C
- **Actions**: Bouger le trieur vers sortie C
- **Résultat attendu**: Trieur en position C
- **Critères de succès**: Position_trieur = C

##### TEST-TRI-008: Validation Colis Trié

- **Préconditions**: Mouvement trieur terminé
- **Actions**: Vérifier le statut de tri
- **Résultat attendu**: Colis_trié = true
- **Critères de succès**: Variable mise à jour

##### TEST-TRI-009: Temps de Tri

- **Préconditions**: Info lu disponible
- **Actions**: Mesurer le temps de tri complet
- **Résultat attendu**: Temps < temps_max_défini
- **Critères de succès**: Performance acceptable

##### TEST-TRI-010: Tri Séquentiel de 5 Colis

- **Préconditions**: 5 colis avec directions différentes
- **Actions**: Trier les 5 colis
- **Résultat attendu**: Tous triés correctement
- **Critères de succès**: 5/5 colis dans bonnes sorties

---

## TESTS D'INTÉGRATION

### INTÉGRATION 1: Ordonnanceur + Check Connect

#### TEST-INT-001: Transition Complète

- **Objectif**: Vérifier la communication entre ordonnanceur et check connect
- **Actions**:
  1. Démarrer ordonnanceur
  2. Déclencher check connect
  3. Vérifier retour Is_Connect_OK
- **Résultat attendu**: Ordonnanceur reçoit correctement le statut
- **Critères de succès**: Transition fluide sans perte de données

### INTÉGRATION 2: Check Connect + Init Position

#### TEST-INT-002: Passage de Données

- **Objectif**: Vérifier le passage des informations de connexion
- **Actions**:
  1. Check connect réussi
  2. Lancer init position
  3. Vérifier utilisation des données de connexion
- **Résultat attendu**: Init position utilise les bonnes connexions
- **Critères de succès**: Pas de reconnexion nécessaire

### INTÉGRATION 3: Init Position + Production

#### TEST-INT-003: Démarrage Production

- **Objectif**: Vérifier le démarrage de production après init
- **Actions**:
  1. Init position terminée
  2. Démarrer production
  3. Vérifier utilisation des paramètres chargés
- **Résultat attendu**: Production démarre avec bons paramètres
- **Critères de succès**: Vitesse et positions correctes

### INTÉGRATION 4: Production + Trier Colis

#### TEST-INT-004: Flux de Données RFID

- **Objectif**: Vérifier le passage des données de lecture au trieur
- **Actions**:
  1. Lecture RFID en production
  2. Transmission au trieur
  3. Tri effectué
- **Résultat attendu**: Données transmises sans perte
- **Critères de succès**: Tri correspond aux données lues

### INTÉGRATION 5: Gestion Erreurs + Tous Modules

#### TEST-INT-005: Erreur en Check Connect

- **Objectif**: Vérifier gestion erreur pendant check connect
- **Actions**:
  1. Simuler échec connexion
  2. Vérifier intervention gestion erreurs
  3. Vérifier retour ordonnanceur
- **Résultat attendu**: Erreur gérée, système en sécurité
- **Critères de succès**: Pas de crash, état cohérent

#### TEST-INT-006: Erreur en Production

- **Objectif**: Vérifier gestion erreur pendant production
- **Actions**:
  1. Simuler erreur RFID
  2. Vérifier arrêt tapis
  3. Vérifier tentative récupération
- **Résultat attendu**: Production arrêtée proprement
- **Critères de succès**: Pas de colis perdu

---

## TESTS DE L'APPLICATION COMPLÈTE

### TEST SYSTÈME 1: Cycle Complet Nominal

#### TEST-SYS-001: Démarrage à Arrêt

- **Objectif**: Tester un cycle complet du système
- **Durée estimée**: 10 minutes
- **Préconditions**:
  - Système éteint
  - Tous équipements connectés
  - 10 colis préparés avec tags RFID
- **Procédure**:
  1. Démarrer le système
  2. Vérifier passage par tous les états (Attente → Check → Init → Production)
  3. Placer 10 colis sur le tapis
  4. Observer le tri complet
  5. Appuyer sur bouton d'arrêt
  6. Vérifier arrêt propre
- **Résultats attendus**:
  - Tous les états traversés sans erreur
  - 10 colis triés correctement
  - Arrêt propre du système
- **Critères de succès**: 100% de réussite, temps < 10 min

### TEST SYSTÈME 2: Performance et Charge

#### TEST-SYS-002: Charge Normale (50 colis/heure)

- **Objectif**: Vérifier performance en charge normale
- **Durée estimée**: 1 heure
- **Actions**: Traiter 50 colis en 1 heure
- **Résultats attendus**:
  - 50/50 colis triés correctement
  - Temps moyen/colis < 72 secondes
  - Aucune erreur système
- **Critères de succès**: Taux de réussite > 98%

#### TEST-SYS-003: Charge Maximale (100 colis/heure)

- **Objectif**: Vérifier performance en charge maximale
- **Durée estimée**: 1 heure
- **Actions**: Traiter 100 colis en 1 heure
- **Résultats attendus**:
  - > 95 colis triés correctement
  - Temps moyen/colis < 36 secondes
  - Erreurs gérées correctement
- **Critères de succès**: Taux de réussite > 95%

### TEST SYSTÈME 3: Robustesse et Fiabilité

#### TEST-SYS-004: Fonctionnement Continu 8 heures

- **Objectif**: Vérifier stabilité sur longue durée
- **Durée estimée**: 8 heures
- **Actions**: Faire fonctionner le système 8h en continu
- **Résultats attendus**:
  - Aucun crash système
  - Performances stables
  - Mémoire stable (pas de fuite)
- **Critères de succès**: Fonctionnement sans interruption

#### TEST-SYS-005: Redémarrage après Erreur Critique

- **Objectif**: Vérifier récupération après erreur critique
- **Actions**:
  1. Système en production
  2. Simuler erreur critique (déconnexion moteur)
  3. Reconnecter équipement
  4. Redémarrer système
- **Résultats attendus**: Redémarrage réussi, reprise production
- **Critères de succès**: Système opérationnel en < 5 min

### TEST SYSTÈME 4: Cas Limites et Erreurs

#### TEST-SYS-006: Colis sans RFID

- **Objectif**: Vérifier gestion colis défectueux
- **Actions**: Introduire 5 colis sans tag RFID parmi 20 colis normaux
- **Résultats attendus**:
  - 20 colis normaux triés
  - 5 colis défectueux détectés et isolés
  - Production continue
- **Critères de succès**: Aucun colis perdu, erreurs loggées

#### TEST-SYS-007: Perte Connexion Dolibarr

- **Objectif**: Vérifier gestion perte connexion externe
- **Actions**:
  1. Système en production
  2. Couper connexion Dolibarr
  3. Observer réaction système
- **Résultats attendus**:
  - Erreur détectée
  - Tentative reconnexion automatique
  - Si échec: mise en sécurité
- **Critères de succès**: Pas de perte de données

#### TEST-SYS-008: Blocage Mécanique Trieur

- **Objectif**: Vérifier détection blocage mécanique
- **Actions**:
  1. Bloquer mécaniquement le trieur
  2. Tenter de trier un colis
- **Résultats attendus**:
  - Timeout détecté
  - Erreur critique levée
  - Tapis arrêté
- **Critères de succès**: Sécurité assurée, pas de dommage

### TEST SYSTÈME 5: Interface et Monitoring

#### TEST-SYS-009: Logs et Traçabilité

- **Objectif**: Vérifier la qualité des logs
- **Actions**: Exécuter cycle complet et analyser logs
- **Résultats attendus**:
  - Tous les événements loggés
  - Timestamps corrects
  - Niveau de log approprié
- **Critères de succès**: Logs exploitables pour diagnostic

#### TEST-SYS-010: Bouton d'Arrêt d'Urgence

- **Objectif**: Vérifier arrêt d'urgence
- **Actions**: Appuyer sur BP_Stop pendant production
- **Résultats attendus**:
  - Arrêt immédiat (< 2 secondes)
  - Tapis et trieur arrêtés
  - État sauvegardé
- **Critères de succès**: Arrêt immédiat et sûr

---

## PROCÉDURES D'INTERVENTION

### INTERVENTION 1: Erreur de Connexion

#### Symptômes

- Is_Connect_OK = false
- Message d'erreur de connexion

#### Diagnostic

1. Vérifier connexion réseau
2. Tester ping vers équipements
3. Vérifier logs de connexion

#### Actions Correctives

1. **Si Dolibarr**: Vérifier serveur, credentials, réseau
2. **Si Moteur**: Vérifier câblage, alimentation, driver
3. **Si Trieur**: Vérifier câblage, alimentation, position
4. **Si RFID**: Vérifier antenne, câblage, configuration

#### Redémarrage

1. Corriger le problème identifié
2. Relancer Check Connect
3. Vérifier Is_Connect_OK = true

### INTERVENTION 2: Erreur d'Initialisation

#### Symptômes

- Is_Prod_OK = false
- Erreur lors de Init Position

#### Diagnostic

1. Vérifier chargement paramètres
2. Vérifier position initiale trieur
3. Consulter logs d'initialisation

#### Actions Correctives

1. **Paramètres non chargés**: Vérifier fichier config, recharger
2. **Trieur bloqué**: Vérifier mécaniquement, débloquer, réinitialiser
3. **Timeout**: Augmenter timeout si nécessaire, vérifier vitesse

#### Redémarrage

1. Corriger le problème
2. Relancer Init Position
3. Vérifier Is_Prod_OK = true

### INTERVENTION 3: Erreur en Production

#### Symptômes

- Production arrêtée
- Erreur RFID ou tri

#### Diagnostic

1. Identifier type d'erreur (logs)
2. Vérifier état équipements
3. Vérifier colis en cours

#### Actions Correctives

1. **RFID illisible**: Retirer colis, nettoyer lecteur, tester
2. **Trieur bloqué**: Débloquer, vérifier mécanique, réinitialiser
3. **Tapis arrêté**: Vérifier moteur, redémarrer

#### Redémarrage

1. Retirer colis problématique si nécessaire
2. Reset module concerné
3. Relancer production

### INTERVENTION 4: Erreur Critique

#### Symptômes

- Système en mise en sécurité
- Attente action opérateur

#### Diagnostic

1. Consulter logs erreur critique
2. Identifier cause racine
3. Évaluer dommages potentiels

#### Actions Correctives

1. **Reset**: Si erreur temporaire, reset système
2. **Diagnostic**: Si cause inconnue, lancer diagnostic complet
3. **Arrêt**: Si problème majeur, arrêter et appeler maintenance

#### Redémarrage

1. Corriger problème identifié
2. Effectuer test de vérification
3. Redémarrer depuis Attente

---

## GÉNÉRATION ET EXÉCUTION DES TESTS

### Structure des Tests Automatisés

#### Organisation des Fichiers

```
tests/
├── unit/
│   ├── test_ordonnanceur.py
│   ├── test_check_connect.py
│   ├── test_init_position.py
│   ├── test_production.py
│   ├── test_gestion_erreurs.py
│   └── test_trier_colis.py
├── integration/
│   ├── test_ordonnanceur_check.py
│   ├── test_check_init.py
│   ├── test_init_production.py
│   ├── test_production_trieur.py
│   └── test_erreurs_integration.py
├── system/
│   ├── test_cycle_complet.py
│   ├── test_performance.py
│   ├── test_robustesse.py
│   └── test_cas_limites.py
├── fixtures/
│   ├── mock_dolibarr.py
│   ├── mock_moteur.py
│   ├── mock_trieur.py
│   └── mock_rfid.py
└── utils/
    ├── test_helpers.py
    └── assertions.py
```

### Framework de Test Recommandé

#### Pour Python

- **pytest**: Framework principal
- **pytest-mock**: Pour les mocks
- **pytest-timeout**: Pour les timeouts
- **pytest-cov**: Pour la couverture de code

#### Pour C/C++

- **Google Test**: Framework principal
- **Google Mock**: Pour les mocks
- **CTest**: Intégration CMake

### Commandes d'Exécution

#### Tests Unitaires

```bash
# Tous les tests unitaires
pytest tests/unit/ -v

# Module spécifique
pytest tests/unit/test_ordonnanceur.py -v

# Test spécifique
pytest tests/unit/test_ordonnanceur.py::test_etat_initial -v
```

#### Tests d'Intégration

```bash
# Tous les tests d'intégration
pytest tests/integration/ -v

# Avec timeout de 60s par test
pytest tests/integration/ -v --timeout=60
```

#### Tests Système

```bash
# Tests système (plus longs)
pytest tests/system/ -v --timeout=600

# Test de performance uniquement
pytest tests/system/test_performance.py -v
```

#### Couverture de Code

```bash
# Générer rapport de couverture
pytest tests/ --cov=src --cov-report=html

# Voir rapport
open htmlcov/index.html
```

### Template de Test Unitaire

```python
import pytest
from unittest.mock import Mock, patch
from src.ordonnanceur import Ordonnanceur

class TestOrdonnanceur:
    """Tests unitaires pour le module Ordonnanceur"""

    @pytest.fixture
    def ordonnanceur(self):
        """Fixture pour créer une instance d'ordonnanceur"""
        return Ordonnanceur()

    def test_etat_initial(self, ordonnanceur):
        """TEST-ORD-001: État Initial"""
        # Arrange
        # (déjà fait par fixture)

        # Act
        etat = ordonnanceur.get_etat()

        # Assert
        assert etat == "Attente", "L'état initial doit être Attente"

    def test_transition_attente_check_connect(self, ordonnanceur):
        """TEST-ORD-002: Transition Attente → Check Connect"""
        # Arrange
        assert ordonnanceur.get_etat() == "Attente"

        # Act
        ordonnanceur.declencher_check_connect()

        # Assert
        assert ordonnanceur.get_etat() == "Check Connect"

    def test_condition_is_connection_true(self, ordonnanceur):
        """TEST-ORD-003: Condition Is_Connection = true"""
        # Arrange
        ordonnanceur.set_etat("Check Connect")
        ordonnanceur.set_is_connection(True)

        # Act
        ordonnanceur.evaluer_transition()

        # Assert
        assert ordonnanceur.get_etat() == "Init Positions"
```

### Template de Test d'Intégration

```python
import pytest
from src.ordonnanceur import Ordonnanceur
from src.check_connect import CheckConnect

class TestIntegrationOrdonnanceurCheck:
    """Tests d'intégration Ordonnanceur + Check Connect"""

    @pytest.fixture
    def systeme(self):
        """Fixture pour créer le système complet"""
        ordonnanceur = Ordonnanceur()
        check_connect = CheckConnect()
        ordonnanceur.set_check_connect(check_connect)
        return ordonnanceur, check_connect

    def test_transition_complete(self, systeme):
        """TEST-INT-001: Transition Complète"""
        # Arrange
        ordonnanceur, check_connect = systeme

        # Act
        ordonnanceur.demarrer()
        ordonnanceur.declencher_check_connect()
        resultat = check_connect.verifier_connexions()

        # Assert
        assert resultat.is_connect_ok == True
        assert ordonnanceur.get_etat() == "Init Positions"
```

### Template de Test Système

```python
import pytest
import time
from src.systeme_complet import SystemeIOT

class TestSystemeComplet:
    """Tests système pour l'application complète"""

    @pytest.fixture
    def systeme(self):
        """Fixture pour créer le système complet"""
        systeme = SystemeIOT()
        systeme.initialiser()
        yield systeme
        systeme.arreter()

    @pytest.mark.slow
    def test_cycle_complet_nominal(self, systeme):
        """TEST-SYS-001: Cycle Complet Nominal"""
        # Arrange
        colis = [self.creer_colis_test(i) for i in range(10)]

        # Act
        debut = time.time()
        systeme.demarrer()

        for colis in colis:
            systeme.ajouter_colis(colis)

        systeme.attendre_fin_traitement()
        duree = time.time() - debut

        # Assert
        assert systeme.get_nb_colis_tries() == 10
        assert duree < 600  # 10 minutes max
        assert systeme.get_nb_erreurs() == 0

    def creer_colis_test(self, id):
        """Créer un colis de test"""
        return {
            'id': id,
            'rfid': f'RFID_{id:04d}',
            'destination': ['A', 'B', 'C'][id % 3]
        }
```

### Exécution Continue (CI/CD)

#### Configuration GitHub Actions

```yaml
name: Tests IOT

on: [push, pull_request]

jobs:
  test:
    runs-on: ubuntu-latest

    steps:
      - uses: actions/checkout@v2

      - name: Setup Python
        uses: actions/setup-python@v2
        with:
          python-version: '3.9'

      - name: Install dependencies
        run: |
          pip install -r requirements.txt
          pip install pytest pytest-cov pytest-timeout

      - name: Run unit tests
        run: pytest tests/unit/ -v --cov=src

      - name: Run integration tests
        run: pytest tests/integration/ -v --timeout=60

      - name: Upload coverage
        uses: codecov/codecov-action@v2
```

### Rapport de Test

#### Format de Rapport

```
RAPPORT DE TEST - [DATE]
========================

Module: [NOM_MODULE]
Version: [VERSION]
Testeur: [NOM]

RÉSUMÉ
------
Tests exécutés: XX
Tests réussis: XX
Tests échoués: XX
Taux de réussite: XX%

DÉTAILS
-------
[TEST-XXX-001] Nom du test: ✓ PASS
[TEST-XXX-002] Nom du test: ✗ FAIL
  Erreur: [Description]

COUVERTURE DE CODE
------------------
Module A: XX%
Module B: XX%
Global: XX%

RECOMMANDATIONS
---------------
- [Recommandation 1]
- [Recommandation 2]
```

---

## ANNEXES

### Checklist Pré-Test

- [ ] Tous les équipements sont connectés
- [ ] Alimentation électrique stable
- [ ] Réseau opérationnel
- [ ] Dolibarr accessible
- [ ] Colis de test préparés
- [ ] Logs activés
- [ ] Espace disque suffisant

### Checklist Post-Test

- [ ] Logs sauvegardés
- [ ] Résultats documentés
- [ ] Anomalies reportées
- [ ] Équipements arrêtés proprement
- [ ] Zone de test nettoyée

### Contacts Urgence

- **Support Technique**: [CONTACT]
- **Maintenance**: [CONTACT]
- **Chef de Projet**: [CONTACT]

### Glossaire

- **BP_Stop**: Bouton Poussoir d'arrêt
- **RFID**: Radio Frequency Identification
- **Is_Connect_OK**: Variable booléenne indiquant connexions OK
- **Is_Prod_OK**: Variable booléenne indiquant production OK
- **Dolibarr**: Système ERP/CRM

---

**Document créé le**: [DATE]
**Version**: 1.0
**Auteur**: Équipe Test IOT
**Statut**: Draft
