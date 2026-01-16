# Structure des Fichiers pour le Projet IOT Tapis Roulant

## Table des Matières

1. [Structure Globale](#structure-globale)
2. [Dossiers Principaux](#dossiers-principaux)
   - [src/](#src)
   - [include/](#include)
   - [lib/](#lib)
   - [test/](#test)
   - [docs/](#docs)
3. [Fichiers Principaux](#fichiers-principaux)
   - [main.cpp](#maincpp)
   - [tapis.cpp](#tapiscpp)
   - [En-têtes](#en-têtes)
4. [Modules](#modules)
   - [Ordonnanceur](#ordonnanceur)
   - [Check Connect](#check-connect)
   - [Init Position](#init-position)
   - [Production](#production)
   - [Gestion des Erreurs](#gestion-des-erreurs)
   - [Trier Colis](#trier-colis)
5. [Fichiers de Configuration](#fichiers-de-configuration)
6. [Fichiers de Test](#fichiers-de-test)
7. [Documentation](#documentation)

---

## Structure Globale

```
/
├── include/
│   └── tapis/
│       ├── tapis.h
│       ├── ordonnanceur.h
│       ├── check_connect.h
│       ├── init_position.h
│       ├── production.h
│       ├── gestion_erreurs.h
│       └── trier_colis.h
├── lib/
│   └── (bibliothèques externes)
├── src/
│   ├── tapis/
│   │   ├── tapis.cpp
│   │   ├── ordonnanceur.cpp
│   │   ├── check_connect.cpp
│   │   ├── init_position.cpp
│   │   ├── production.cpp
│   │   ├── gestion_erreurs.cpp
│   │   └── trier_colis.cpp
│   └── main.cpp
├── test/
│   ├── unit/
│   │   ├── test_ordonnanceur.cpp
│   │   ├── test_check_connect.cpp
│   │   ├── test_init_position.cpp
│   │   ├── test_production.cpp
│   │   ├── test_gestion_erreurs.cpp
│   │   └── test_trier_colis.cpp
│   ├── integration/
│   │   ├── test_ordonnanceur_check.cpp
│   │   ├── test_check_init.cpp
│   │   ├── test_init_production.cpp
│   │   ├── test_production_trieur.cpp
│   │   └── test_erreurs_integration.cpp
│   └── system/
│       ├── test_cycle_complet.cpp
│       ├── test_performance.cpp
│       ├── test_robustesse.cpp
│       └── test_cas_limites.cpp
├── docs/
│   ├── ANALYSE_GENERALE.md
│   ├── STRUCTURE_FICHIERS.md
│   ├── PROTOCOLE_TESTS.md
│   ├── Flow Chart.drawio
│   └── T-IOT-901_project.pdf
├── platformio.ini
└── README.md
```

---

## Dossiers Principaux

### src/

Ce dossier contient le code source principal du projet.

- **main.cpp** : Point d'entrée principal du programme.
- **tapis/** : Dossier contenant les modules spécifiques au tapis roulant.

### include/

Ce dossier contient les fichiers d'en-tête (.h) pour les modules.

- **tapis/** : Dossier contenant les en-têtes des modules spécifiques au tapis roulant.

### lib/

Ce dossier contient les bibliothèques externes nécessaires au projet.

### test/

Ce dossier contient les tests unitaires, d'intégration et système.

- **unit/** : Tests unitaires pour chaque module.
- **integration/** : Tests d'intégration pour les interactions entre modules.
- **system/** : Tests système pour l'application complète.

### docs/

Ce dossier contient la documentation du projet.

- **ANALYSE_GENERALE.md** : Analyse générale du projet.
- **STRUCTURE_FICHIERS.md** : Structure des fichiers du projet.
- **PROTOCOLE_TESTS.md** : Protocole de tests.
- **Flow Chart.drawio** : Diagramme de flux.
- **T-IOT-901_project.pdf** : Documentation du projet.

---

## Fichiers Principaux

### main.cpp

**Description** : Point d'entrée principal du programme.

**Contenu** :
- Initialisation du système
- Configuration des modules
- Boucle principale
- Gestion des erreurs globales

### tapis.cpp

**Description** : Module principal du tapis roulant.

**Contenu** :
- Initialisation du tapis
- Gestion des mouvements
- Communication avec les autres modules

### En-têtes

Les fichiers d'en-tête (.h) définissent les interfaces des modules.

- **tapis.h** : Interface du module tapis
- **ordonnanceur.h** : Interface du module ordonnanceur
- **check_connect.h** : Interface du module check connect
- **init_position.h** : Interface du module init position
- **production.h** : Interface du module production
- **gestion_erreurs.h** : Interface du module gestion des erreurs
- **trier_colis.h** : Interface du module trier colis

---

## Modules

### Ordonnanceur

**Fichiers** :
- **src/tapis/ordonnanceur.cpp**
- **include/tapis/ordonnanceur.h**

**Description** : Gère les états du système et les transitions.

**Fonctionnalités** :
- États : Attente, Check Connect, Init Positions, Production
- Transitions entre états
- Gestion des conditions : Is_Connection, Is_Prod_Ok, BP_Stop, Erreur_Critique

### Check Connect

**Fichiers** :
- **src/tapis/check_connect.cpp**
- **include/tapis/check_connect.h**

**Description** : Vérifie les connexions des équipements.

**Fonctionnalités** :
- Test de connexion à Dolibarr
- Test de connexion au moteur du tapis
- Test de connexion au trieur
- Test de connexion au capteur RFID
- Validation Is_Connect_OK

### Init Position

**Fichiers** :
- **src/tapis/init_position.cpp**
- **include/tapis/init_position.h**

**Description** : Initialise les positions des équipements.

**Fonctionnalités** :
- Vérification Is_Connect
- Chargement des paramètres du moteur
- Initialisation de la position du trieur
- Validation Is_Prod_Ok

### Production

**Fichiers** :
- **src/tapis/production.cpp**
- **include/tapis/production.h**

**Description** : Implémente le cycle de production.

**Fonctionnalités** :
- Vérification Is_Prod
- Démarrage du tapis roulant
- Lecture des informations RFID des colis
- Tri des colis
- Gestion du bouton d'arrêt

### Gestion des Erreurs

**Fichiers** :
- **src/tapis/gestion_erreurs.cpp**
- **include/tapis/gestion_erreurs.h**

**Description** : Détecte et gère les erreurs.

**Fonctionnalités** :
- Détection des erreurs
- Classification des erreurs (critique/non critique)
- Auto-récupération
- Mise en sécurité
- Attente action opérateur

### Trier Colis

**Fichiers** :
- **src/tapis/trier_colis.cpp**
- **include/tapis/trier_colis.h**

**Description** : Implémente le système de tri.

**Fonctionnalités** :
- Réception des informations du colis
- Définition de la direction de tri
- Mouvement du trieur vers la bonne sortie
- Validation du tri

---

## Fichiers de Configuration

**Fichiers** :
- **platformio.ini** : Configuration du projet PlatformIO

**Description** : Ce fichier contient la configuration nécessaire pour compiler et déployer le projet sur la plateforme cible.

---

## Fichiers de Test

### Tests Unitaires

**Dossier** : test/unit/

**Fichiers** :
- **test_ordonnanceur.cpp** : Tests unitaires pour le module ordonnanceur
- **test_check_connect.cpp** : Tests unitaires pour le module check connect
- **test_init_position.cpp** : Tests unitaires pour le module init position
- **test_production.cpp** : Tests unitaires pour le module production
- **test_gestion_erreurs.cpp** : Tests unitaires pour le module gestion des erreurs
- **test_trier_colis.cpp** : Tests unitaires pour le module trier colis

### Tests d'Intégration

**Dossier** : test/integration/

**Fichiers** :
- **test_ordonnanceur_check.cpp** : Tests d'intégration entre ordonnanceur et check connect
- **test_check_init.cpp** : Tests d'intégration entre check connect et init position
- **test_init_production.cpp** : Tests d'intégration entre init position et production
- **test_production_trieur.cpp** : Tests d'intégration entre production et trier colis
- **test_erreurs_integration.cpp** : Tests d'intégration pour la gestion des erreurs

### Tests Système

**Dossier** : test/system/

**Fichiers** :
- **test_cycle_complet.cpp** : Tests système pour un cycle complet
- **test_performance.cpp** : Tests système pour la performance
- **test_robustesse.cpp** : Tests système pour la robustesse
- **test_cas_limites.cpp** : Tests système pour les cas limites

---

## Documentation

**Dossier** : docs/

**Fichiers** :
- **ANALYSE_GENERALE.md** : Analyse générale du projet
- **STRUCTURE_FICHIERS.md** : Structure des fichiers du projet
- **PROTOCOLE_TESTS.md** : Protocole de tests
- **Flow Chart.drawio** : Diagramme de flux
- **T-IOT-901_project.pdf** : Documentation du projet

---

**Document créé le**: [DATE]
**Version**: 1.0
**Auteur**: Équipe Développement IOT
**Statut**: Draft