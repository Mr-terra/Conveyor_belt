# General Analysis - IoT Conveyor Belt Project

> **Note:** this is the initial design analysis, kept for reference. It predates the hardware
> abstraction layer and several behaviour changes. For the current behaviour, see the
> [README](../README.md), [SYSTEM_USAGE.md](SYSTEM_USAGE.md) and [state_management_error.md](state_management_error.md).

## Table of Contents

1. [Understanding the Global Architecture](#understanding-the-global-architecture)
2. [Development Steps](#development-steps)
   - [Step 1: Scheduler](#step-1-scheduler)
   - [Step 2: Check Connect](#step-2-check-connect)
   - [Step 3: Init Position](#step-3-init-position)
   - [Step 4: Production](#step-4-production)
   - [Step 5: Error Management](#step-5-error-management)
   - [Step 6: Sort Parcels](#step-6-sort-parcels)
3. [Integration and Tests](#integration-and-tests)
   - [Integration](#integration)
   - [Tests](#tests)
4. [Documentation and Maintenance](#documentation-and-maintenance)
5. [Execution and Monitoring](#execution-and-monitoring)

---

## Understanding the Global Architecture

The system is made of several main modules:
- **Scheduler**: Manages the system states and transitions
- **Check Connect**: Checks the connections of the devices
- **Init Position**: Initializes the positions of the devices
- **Production**: Main production cycle
- **Error Management**: Error detection and recovery
- **Sort Parcels**: Parcel sorting system

---

## Development Steps

### Step 1: Scheduler

**Goal**: Implement the finite state machine

**Tasks**:
- Create the states: Waiting, Check Connect, Init Positions, Production
- Implement the transitions between states
- Handle the conditions: Is_Connection, Is_Prod_Ok, BP_Stop, Erreur_Critique
- Implement the full loop

### Step 2: Check Connect

**Goal**: Check the connections of the devices

**Tasks**:
- Test the connection to Dolibarr
- Test the connection to the conveyor motor
- Test the connection to the sorter
- Test the connection to the RFID sensor
- Validate Is_Connect_OK

### Step 3: Init Position

**Goal**: Initialize the positions of the devices

**Tasks**:
- Check Is_Connect
- Load the motor parameters
- Initialize the sorter position
- Validate Is_Prod_Ok

### Step 4: Production

**Goal**: Implement the production cycle

**Tasks**:
- Check Is_Prod
- Start the conveyor belt
- Read the RFID information of the parcels
- Sort the parcels
- Handle the stop button

### Step 5: Error Management

**Goal**: Detect and handle errors

**Tasks**:
- Detect errors
- Classify errors (critical / non-critical)
- Implement auto-recovery
- Put the system in a safe state
- Wait for operator action

### Step 6: Sort Parcels

**Goal**: Implement the sorting system

**Tasks**:
- Receive the parcel information
- Set the sorting direction
- Move the sorter to the right output
- Validate the sort

---

## Integration and Tests

### Integration

- **Scheduler + Check Connect**: Verify the communication between the modules
- **Check Connect + Init Position**: Verify the passing of the connection information
- **Init Position + Production**: Verify the start of production
- **Production + Sort Parcels**: Verify the RFID data flow
- **Error Management + All Modules**: Verify error handling

### Tests

- **Unit Tests**: Test each module individually
- **Integration Tests**: Test the interaction between the modules
- **System Tests**: Test the complete application

---

## Documentation and Maintenance

- **Documentation**: Document each module and function
- **Logs**: Implement a logging system for monitoring and diagnosis
- **Maintenance**: Plan intervention procedures for common errors

---

## Execution and Monitoring

- **Execution**: Deploy and run the system
- **Monitoring**: Monitor the system in real time
- **Continuous Improvement**: Optimize and improve the system based on feedback

---

**Document created on**: [DATE]
**Version**: 1.0
**Author**: IoT Development Team
**Status**: Draft
