# File Structure for the IoT Conveyor Belt Project

## Table of Contents

1. [Global Structure](#global-structure)
2. [Main Folders](#main-folders)
   - [src/](#src)
   - [include/](#include)
   - [lib/](#lib)
   - [test/](#test)
   - [docs/](#docs)
3. [Main Files](#main-files)
   - [main.cpp](#maincpp)
   - [tapis.cpp](#tapiscpp)
   - [Headers](#headers)
4. [Modules](#modules)
   - [Scheduler](#scheduler)
   - [Check Connect](#check-connect)
   - [Init Position](#init-position)
   - [Production](#production)
   - [Error Management](#error-management)
   - [Sort Parcels](#sort-parcels)
5. [Configuration Files](#configuration-files)
6. [Test Files](#test-files)
7. [Documentation](#documentation)

---

## Global Structure

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
│   └── (external libraries)
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
│   ├── GENERAL_ANALYSIS.md
│   ├── FILE_STRUCTURE.md
│   ├── TEST_PROTOCOL.md
│   ├── Flow Chart.drawio
│   └── T-IOT-901_project.pdf
├── platformio.ini
└── README.md
```

---

## Main Folders

### src/

This folder contains the main source code of the project.

- **main.cpp**: Main entry point of the program.
- **tapis/**: Folder containing the modules specific to the conveyor belt.

### include/

This folder contains the header files (.h) for the modules.

- **tapis/**: Folder containing the headers of the modules specific to the conveyor belt.

### lib/

This folder contains the external libraries required by the project.

### test/

This folder contains the unit, integration and system tests.

- **unit/**: Unit tests for each module.
- **integration/**: Integration tests for the interactions between modules.
- **system/**: System tests for the complete application.

### docs/

This folder contains the project documentation.

- **GENERAL_ANALYSIS.md**: General analysis of the project.
- **FILE_STRUCTURE.md**: File structure of the project.
- **TEST_PROTOCOL.md**: Test protocol.
- **Flow Chart.drawio**: Flow diagram.
- **T-IOT-901_project.pdf**: Project documentation.

---

## Main Files

### main.cpp

**Description**: Main entry point of the program.

**Content**:
- System initialization
- Module configuration
- Main loop
- Global error handling

### tapis.cpp

**Description**: Main module of the conveyor belt.

**Content**:
- Conveyor initialization
- Movement handling
- Communication with the other modules

### Headers

The header files (.h) define the interfaces of the modules.

- **tapis.h**: Interface of the conveyor module
- **ordonnanceur.h**: Interface of the scheduler module
- **check_connect.h**: Interface of the check connect module
- **init_position.h**: Interface of the init position module
- **production.h**: Interface of the production module
- **gestion_erreurs.h**: Interface of the error management module
- **trier_colis.h**: Interface of the sort parcels module

---

## Modules

### Scheduler

**Files**:
- **src/tapis/ordonnanceur.cpp**
- **include/tapis/ordonnanceur.h**

**Description**: Manages the system states and transitions.

**Features**:
- States: Waiting, Check Connect, Init Positions, Production
- Transitions between states
- Condition handling: Is_Connection, Is_Prod_Ok, BP_Stop, Erreur_Critique

### Check Connect

**Files**:
- **src/tapis/check_connect.cpp**
- **include/tapis/check_connect.h**

**Description**: Checks the connections of the devices.

**Features**:
- Connection test to Dolibarr
- Connection test to the conveyor motor
- Connection test to the sorter
- Connection test to the RFID sensor
- Validation of Is_Connect_OK

### Init Position

**Files**:
- **src/tapis/init_position.cpp**
- **include/tapis/init_position.h**

**Description**: Initializes the positions of the devices.

**Features**:
- Is_Connect check
- Loading of the motor parameters
- Initialization of the sorter position
- Validation of Is_Prod_Ok

### Production

**Files**:
- **src/tapis/production.cpp**
- **include/tapis/production.h**

**Description**: Implements the production cycle.

**Features**:
- Is_Prod check
- Starting the conveyor belt
- Reading the RFID information of the parcels
- Sorting the parcels
- Handling the stop button

### Error Management

**Files**:
- **src/tapis/gestion_erreurs.cpp**
- **include/tapis/gestion_erreurs.h**

**Description**: Detects and handles errors.

**Features**:
- Error detection
- Error classification (critical / non-critical)
- Auto-recovery
- Putting the system in a safe state
- Waiting for operator action

### Sort Parcels

**Files**:
- **src/tapis/trier_colis.cpp**
- **include/tapis/trier_colis.h**

**Description**: Implements the sorting system.

**Features**:
- Receiving the parcel information
- Setting the sorting direction
- Moving the sorter to the right output
- Validating the sort

---

## Configuration Files

**Files**:
- **platformio.ini**: PlatformIO project configuration

**Description**: This file contains the configuration needed to build and deploy the project to the target platform.

---

## Test Files

### Unit Tests

**Folder**: test/unit/

**Files**:
- **test_ordonnanceur.cpp**: Unit tests for the scheduler module
- **test_check_connect.cpp**: Unit tests for the check connect module
- **test_init_position.cpp**: Unit tests for the init position module
- **test_production.cpp**: Unit tests for the production module
- **test_gestion_erreurs.cpp**: Unit tests for the error management module
- **test_trier_colis.cpp**: Unit tests for the sort parcels module

### Integration Tests

**Folder**: test/integration/

**Files**:
- **test_ordonnanceur_check.cpp**: Integration tests between the scheduler and check connect
- **test_check_init.cpp**: Integration tests between check connect and init position
- **test_init_production.cpp**: Integration tests between init position and production
- **test_production_trieur.cpp**: Integration tests between production and sort parcels
- **test_erreurs_integration.cpp**: Integration tests for error management

### System Tests

**Folder**: test/system/

**Files**:
- **test_cycle_complet.cpp**: System tests for a complete cycle
- **test_performance.cpp**: System tests for performance
- **test_robustesse.cpp**: System tests for robustness
- **test_cas_limites.cpp**: System tests for edge cases

---

## Documentation

**Folder**: docs/

**Files**:
- **GENERAL_ANALYSIS.md**: General analysis of the project
- **FILE_STRUCTURE.md**: File structure of the project
- **TEST_PROTOCOL.md**: Test protocol
- **Flow Chart.drawio**: Flow diagram
- **T-IOT-901_project.pdf**: Project documentation

---

**Document created on**: [DATE]
**Version**: 1.0
**Author**: IoT Development Team
**Status**: Draft
