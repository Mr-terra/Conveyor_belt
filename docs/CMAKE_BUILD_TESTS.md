# CMake, Build, Tests et CI/CD

Ce document explique comment fonctionne le système de build avec CMake, comment exécuter les tests, et comment la CI/CD est configurée pour générer les artefacts.

## Table des matières
- [CMake, Build, Tests et CI/CD](#cmake-build-tests-et-cicd)
  - [Table des matières](#table-des-matières)
  - [1. Structure du projet](#1-structure-du-projet)
  - [2. Fichiers CMake](#2-fichiers-cmake)
    - [2.1. Fichier CMakeLists.txt principal](#21-fichier-cmakeliststxt-principal)
    - [2.2. Fichier CMakeLists.txt dans src/](#22-fichier-cmakeliststxt-dans-src)
    - [2.3. Fichier CMakeLists.txt dans test/](#23-fichier-cmakeliststxt-dans-test)
  - [3. Build du projet](#3-build-du-projet)
    - [3.1. Prérequis](#31-prérequis)
    - [3.2. Configuration du build](#32-configuration-du-build)
    - [3.3. Compilation](#33-compilation)
  - [4. Exécution des tests](#4-exécution-des-tests)
    - [4.1. Tests unitaires](#41-tests-unitaires)
    - [4.2. Tests d'intégration](#42-tests-dintégration)
  - [5. Configuration CI/CD](#5-configuration-cicd)
    - [5.1. Fichier workflow CI](#51-fichier-workflow-ci)
    - [5.2. Étapes de la CI](#52-étapes-de-la-ci)
    - [5.3. Génération des artefacts](#53-génération-des-artefacts)
  - [6. Commandes utiles](#6-commandes-utiles)

## 1. Structure du projet

Le projet est organisé comme suit :

```
conveyor/
├── CMakeLists.txt                  # Configuration CMake principale
├── src/
│   └── CMakeLists.txt             # Configuration CMake pour le code source
├── test/
│   ├── CMakeLists.txt             # Configuration CMake pour les tests
│   └── conveyor_tests/
│       └── CMakeLists.txt         # Configuration CMake pour les tests unitaires
├── include/                        # Fichiers d'en-tête
├── lib/                           # Bibliothèques externes
└── workflows/
    └── ci.yml                     # Configuration GitHub Actions CI
```

## 2. Fichiers CMake

### 2.1. Fichier CMakeLists.txt principal

Le fichier `CMakeLists.txt` à la racine du projet configure :

- Le nom du projet et la version
- Le standard C++ utilisé (C++17)
- Les options de build (tests, coverage, etc.)
- L'inclusion des sous-répertoires (src/, test/)
- La configuration des tests avec CTest
- La génération des artefacts

Exemple de configuration :

```cmake
cmake_minimum_required(VERSION 3.10)
project(conveyor VERSION 1.0.0 LANGUAGES CXX)

# Configuration du standard C++
set(CMAKE_CXX_STANDARD 17)
set(CMAKE_CXX_STANDARD_REQUIRED ON)

# Options de build
option(BUILD_TESTS "Build tests" ON)
option(BUILD_COVERAGE "Build with coverage support" OFF)

# Ajout des sous-répertoires
add_subdirectory(src)
add_subdirectory(test)

# Configuration des tests
enable_testing()
include(CTest)
```

### 2.2. Fichier CMakeLists.txt dans src/

Ce fichier configure la compilation du code source principal :

- Création de la bibliothèque ou de l'exécutable principal
- Ajout des fichiers sources
- Configuration des include paths
- Liaison avec les bibliothèques externes

Exemple :

```cmake
# Création de l'exécutable principal
add_executable(conveyor
    main.cpp
    conveyor/conveyor.cpp
    # ... autres fichiers sources
)

# Configuration des include paths
target_include_directories(conveyor PRIVATE
    ${CMAKE_SOURCE_DIR}/include
    ${CMAKE_SOURCE_DIR}/lib/M5Stack_lib/src
)

# Liaison avec les bibliothèques
target_link_libraries(conveyor
    pthread
    # ... autres bibliothèques
)
```

### 2.3. Fichier CMakeLists.txt dans test/

Ce fichier configure la compilation et l'exécution des tests :

- Création des exécutables de test
- Configuration des dépendances de test
- Ajout des tests à CTest

Exemple :

```cmake
# Création de l'exécutable de test unitaire
add_executable(conveyor_unit_test
    conveyor_tests/conveyor_unit_test.cpp
    conveyor_tests/mock_grbl.h
    conveyor_tests/mock_wire.h
)

# Configuration des dépendances
target_link_libraries(conveyor_unit_test
    conveyor
    GTest::GTest
    GTest::Main
)

# Ajout du test à CTest
add_test(NAME conveyor_unit_test COMMAND conveyor_unit_test)
```

## 3. Build du projet

### 3.1. Prérequis

Pour builder le projet, vous avez besoin de :

- CMake (version 3.10 ou supérieure)
- Un compilateur C++17 (GCC, Clang, MSVC)
- Git (pour les dépendances)
- Make ou Ninja (pour la compilation)

### 3.2. Configuration du build

Créez un répertoire de build et configurez le projet :

```bash
mkdir build
cd build
cmake ..
```

Pour activer les tests :

```bash
cmake -DBUILD_TESTS=ON ..
```

Pour activer la couverture de code :

```bash
cmake -DBUILD_COVERAGE=ON ..
```

### 3.3. Compilation

Une fois configuré, compilez le projet :

```bash
cmake --build .
```

Ou avec Make :

```bash
make
```

## 4. Exécution des tests

### 4.1. Tests unitaires

Les tests unitaires sont situés dans le répertoire `test/conveyor_tests/`.

Pour exécuter les tests :

```bash
cd build
ctest
```

Ou pour un test spécifique :

```bash
ctest -R conveyor_unit_test
```

### 4.2. Tests d'intégration

Les tests d'intégration sont situés dans `test/test_integration.cpp`.

Pour exécuter les tests d'intégration :

```bash
./test/test_integration
```

## 5. Configuration CI/CD

### 5.1. Fichier workflow CI

Le fichier `workflows/ci.yml` configure l'intégration continue avec GitHub Actions.

Il contient :

- Les déclencheurs (push, pull request)
- Les jobs de build et test
- La génération des artefacts
- Le déploiement

### 5.2. Étapes de la CI

La CI effectue les étapes suivantes :

1. **Checkout** : Récupération du code source
2. **Installation des dépendances** : Installation de CMake, compilateur, etc.
3. **Configuration** : Exécution de CMake
4. **Build** : Compilation du projet
5. **Tests** : Exécution des tests unitaires et d'intégration
6. **Génération des artefacts** : Création des binaires et archives
7. **Upload des artefacts** : Mise à disposition des artefacts

### 5.3. Génération des artefacts

Les artefacts générés incluent :

- L'exécutable principal (`conveyor`)
- Les exécutables de test
- Les rapports de couverture (si activé)
- Les logs de build et de test

## 6. Commandes utiles

| Commande | Description |
|----------|-------------|
| `cmake ..` | Configure le projet |
| `cmake --build .` | Compile le projet |
| `ctest` | Exécute tous les tests |
| `ctest -V` | Exécute les tests en mode verbeux |
| `ctest -R test_name` | Exécute un test spécifique |
| `make clean` | Nettoie les fichiers de build |
| `cmake -DBUILD_TESTS=OFF ..` | Désactive les tests |
| `cmake -DBUILD_COVERAGE=ON ..` | Active la couverture de code |
