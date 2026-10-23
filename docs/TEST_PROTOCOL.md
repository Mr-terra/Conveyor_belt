# TEST PROTOCOL - IOT CONVEYOR BELT PROJECT

> **Note:** this is the manual test protocol from the school project. The automated
> tests are the host unit tests in `test/host/` (see `CMAKE_BUILD_TESTS.md`). The Python
> file layout further down is historical and does not match the repository.

## TABLE OF CONTENTS

1. [Tests per Module](#tests-per-module)
2. [Integration Tests](#integration-tests)
3. [Full Application Tests](#full-application-tests)
4. [Intervention Procedures](#intervention-procedures)
5. [Test Generation and Execution](#test-generation-and-execution)

---

## TESTS PER MODULE

### MODULE 1: SCHEDULER

#### Goal

Check the management of the system states and the transitions between the phases.

#### Unit Tests

##### TEST-ORD-001: Initial State

- **Preconditions**: System just started
- **Actions**: Start the scheduler
- **Expected result**: State = "Waiting"
- **Success criteria**: State variable = "Waiting"

##### TEST-ORD-002: Transition Waiting → Check Connect

- **Preconditions**: State = "Waiting"
- **Actions**: Trigger the connection check
- **Expected result**: Switch to the "Check Connect" state
- **Success criteria**: State changes correctly

##### TEST-ORD-003: Condition Is_Connection = true

- **Preconditions**: Check Connect completed successfully
- **Actions**: Check the Is_Connection variable
- **Expected result**: Is_Connection = true, switch to "Init Positions"
- **Success criteria**: Correct transition to Init Positions

##### TEST-ORD-004: Condition Is_Prod_Ok = true

- **Preconditions**: Init Positions completed successfully
- **Actions**: Check the Is_Prod_Ok variable
- **Expected result**: Is_Prod_Ok = true, switch to "Production"
- **Success criteria**: Correct transition to Production

##### TEST-ORD-005: BP_Stop Handling

- **Preconditions**: System in production
- **Actions**: Set BP_Stop = true
- **Expected result**: Return to the "Waiting" state
- **Success criteria**: Immediate stop and return to Waiting

##### TEST-ORD-006: Erreur_Critique Handling

- **Preconditions**: System in production
- **Actions**: Trigger Erreur_Critique = true
- **Expected result**: Return to the "Waiting" state
- **Success criteria**: Immediate stop and return to Waiting

##### TEST-ORD-007: Full Loop

- **Preconditions**: System initialized
- **Actions**: Run a full cycle (Waiting → Check → Init → Production → Waiting)
- **Expected result**: Full cycle without error
- **Success criteria**: All the transitions run correctly

---

### MODULE 2: CHECK CONNECT (Connection Check)

#### Goal

Check the connectivity of all the devices in the system.

#### Unit Tests

##### TEST-CHK-001: Dolibarr Connection Test

- **Preconditions**: System in the "Waiting" state
- **Actions**: Test the connection to Dolibarr
- **Expected result**: Connection established, OK returned
- **Success criteria**: Dolibarr connection status = OK

##### TEST-CHK-002: Conveyor Motor Connection Test

- **Preconditions**: Dolibarr connection OK
- **Actions**: Test the connection to the conveyor motor
- **Expected result**: Connection established, OK returned
- **Success criteria**: Motor connection status = OK

##### TEST-CHK-003: Sorter Connection Test

- **Preconditions**: Conveyor motor connection OK
- **Actions**: Test the connection to the sorter
- **Expected result**: Connection established, OK returned
- **Success criteria**: Sorter connection status = OK

##### TEST-CHK-004: RFID Sensor Connection Test

- **Preconditions**: Sorter connection OK
- **Actions**: Test the connection to the RFID sensor
- **Expected result**: Connection established, OK returned
- **Success criteria**: RFID connection status = OK

##### TEST-CHK-005: Is_Connect_OK Validation

- **Preconditions**: All the connection tests passed
- **Actions**: Check the Is_Connect_OK variable
- **Expected result**: Is_Connect_OK = true
- **Success criteria**: Variable correctly updated

##### TEST-CHK-006: Dolibarr Connection Failure

- **Preconditions**: Dolibarr unreachable
- **Actions**: Try to connect
- **Expected result**: Error detected, Is_Connect_OK = false
- **Success criteria**: Error correctly reported

##### TEST-CHK-007: Motor Connection Failure

- **Preconditions**: Motor disconnected
- **Actions**: Try to connect
- **Expected result**: Error detected, Is_Connect_OK = false
- **Success criteria**: Error correctly reported

##### TEST-CHK-008: Sorter Connection Failure

- **Preconditions**: Sorter disconnected
- **Actions**: Try to connect
- **Expected result**: Error detected, Is_Connect_OK = false
- **Success criteria**: Error correctly reported

##### TEST-CHK-009: RFID Connection Failure

- **Preconditions**: RFID sensor disconnected
- **Actions**: Try to connect
- **Expected result**: Error detected, Is_Connect_OK = false
- **Success criteria**: Error correctly reported

##### TEST-CHK-010: Full Check Sequence

- **Preconditions**: All the devices connected
- **Actions**: Run the full sequence
- **Expected result**: All tests pass, Is_Connect_OK = true
- **Success criteria**: Full sequence without error

---

### MODULE 3: INIT POSITION (Position Initialization)

#### Goal

Check the correct initialization of the device positions.

#### Unit Tests

##### TEST-INI-001: Is_Connect Check

- **Preconditions**: Check Connect completed successfully
- **Actions**: Check Is_Connect before initialization
- **Expected result**: Is_Connect = true
- **Success criteria**: Condition validated

##### TEST-INI-002: Loading Conveyor Motor Parameters

- **Preconditions**: Is_Connect = true
- **Actions**: Load the motor parameters
- **Expected result**: Parameters loaded correctly
- **Success criteria**: Parameter data loaded OK = true

##### TEST-INI-003: Parameter Loading Failure

- **Preconditions**: Parameters corrupted or unreachable
- **Actions**: Try to load them
- **Expected result**: Error raised
- **Success criteria**: Error detected and reported

##### TEST-INI-004: Sorter Position Init

- **Preconditions**: Motor parameters loaded
- **Actions**: Initialize the sorter position
- **Expected result**: Sorter in its initial position
- **Success criteria**: Sorter position = init_position

##### TEST-INI-005: Is_Prod_Ok Validation

- **Preconditions**: Sorter position initialization completed
- **Actions**: Check Is_Prod_Ok
- **Expected result**: Is_Prod_Ok = true
- **Success criteria**: Variable correctly updated

##### TEST-INI-006: Full Initialization Sequence

- **Preconditions**: Connections OK
- **Actions**: Run the whole initialization sequence
- **Expected result**: System ready for production
- **Success criteria**: Is_Prod_Ok = true, all the devices initialized

##### TEST-INI-007: Sorter Initialization Timeout

- **Preconditions**: Sorter mechanically blocked
- **Actions**: Start the initialization
- **Expected result**: Timeout detected, error raised
- **Success criteria**: Timeout error reported

---

### MODULE 4: PRODUCTION (Production Cycle)

#### Goal

Check the operation of the full production cycle.

#### Unit Tests

##### TEST-PRD-001: Is_Prod Check

- **Preconditions**: Initialization completed successfully
- **Actions**: Check Is_Prod before starting
- **Expected result**: Is_Prod = true
- **Success criteria**: Condition validated

##### TEST-PRD-002: Starting the Conveyor Belt

- **Preconditions**: Is_Prod = true
- **Actions**: Run the conveyor belt
- **Expected result**: Conveyor belt moving at the configured speed
- **Success criteria**: Belt speed = config_speed

##### TEST-PRD-003: RFID Parcel Reading

- **Preconditions**: Conveyor belt moving, parcel on the belt
- **Actions**: Read the RFID information
- **Expected result**: Information read correctly
- **Success criteria**: Info_read != empty, valid data

##### TEST-PRD-004: Parcel Sorting

- **Preconditions**: Information read
- **Actions**: Sort the parcel according to the information
- **Expected result**: Parcel sorted = true
- **Success criteria**: Parcel directed to the right output

##### TEST-PRD-005: Stop Button (Not Pressed)

- **Preconditions**: Production running
- **Actions**: Check the state of the stop button
- **Expected result**: Button = false, production continues
- **Success criteria**: Cycle continues normally

##### TEST-PRD-006: Stop Button (Pressed)

- **Preconditions**: Production running
- **Actions**: Press the stop button
- **Expected result**: Is_Prod = false, cycle stopped
- **Success criteria**: Production stops immediately

##### TEST-PRD-007: Full Cycle with 1 Parcel

- **Preconditions**: System in production
- **Actions**: Process 1 parcel from start to end
- **Expected result**: Parcel read, sorted and removed
- **Success criteria**: Full cycle without error

##### TEST-PRD-008: Full Cycle with 10 Parcels

- **Preconditions**: System in production
- **Actions**: Process 10 consecutive parcels
- **Expected result**: All the parcels processed correctly
- **Success criteria**: 10 parcels sorted without error

##### TEST-PRD-009: Parcel without RFID

- **Preconditions**: Production running
- **Actions**: A parcel without an RFID tag goes through
- **Expected result**: Error detected, parcel not sorted
- **Success criteria**: Error reported, parcel flagged as faulty

##### TEST-PRD-010: Unreadable RFID

- **Preconditions**: Production running
- **Actions**: Damaged RFID tag
- **Expected result**: Read error detected
- **Success criteria**: Error reported correctly

---

### MODULE 5: ERROR MANAGEMENT

#### Goal

Check the detection and handling of the system errors.

#### Unit Tests

##### TEST-ERR-001: Error Detection

- **Preconditions**: System running
- **Actions**: Simulate an error
- **Expected result**: Error detected
- **Success criteria**: State = "Error detected"

##### TEST-ERR-002: Non-Critical Error Classification

- **Preconditions**: Error detected
- **Actions**: Classify the error (non-critical)
- **Expected result**: Erreur_critique = false
- **Success criteria**: Switch to auto-recovery

##### TEST-ERR-003: Auto-Recovery Retry

- **Preconditions**: Non-critical error
- **Actions**: Attempt a retry
- **Expected result**: Recovery successful
- **Success criteria**: Recovery_successful = true

##### TEST-ERR-004: Auto-Recovery Partial Reset

- **Preconditions**: Non-critical error
- **Actions**: Perform a partial reset
- **Expected result**: Module reset
- **Success criteria**: Recovery_successful = true

##### TEST-ERR-005: Auto-Recovery Parameter Reload

- **Preconditions**: Non-critical error
- **Actions**: Reload the parameters
- **Expected result**: Parameters reloaded
- **Success criteria**: Recovery_successful = true

##### TEST-ERR-006: Auto-Recovery Failure

- **Preconditions**: Non-critical error, 3 failed attempts
- **Actions**: Recovery attempts
- **Expected result**: Reclassified as a critical error
- **Success criteria**: Erreur_critique = true

##### TEST-ERR-007: Return to Original Module

- **Preconditions**: Recovery successful
- **Actions**: Return to the original module
- **Expected result**: Normal operation resumes
- **Success criteria**: State = previous_state

##### TEST-ERR-008: Critical Error Classification

- **Preconditions**: Error detected
- **Actions**: Classify the error (critical)
- **Expected result**: Erreur_critique = true
- **Success criteria**: Switch to safe state

##### TEST-ERR-009: Safe State - Stop Conveyor

- **Preconditions**: Critical error
- **Actions**: Stop the conveyor belt
- **Expected result**: Conveyor belt stopped
- **Success criteria**: Belt_speed = 0

##### TEST-ERR-010: Safe State - Stop Sorter

- **Preconditions**: Critical error
- **Actions**: Stop the sorter
- **Expected result**: Sorter stopped
- **Success criteria**: Sorter_movement = 0

##### TEST-ERR-011: Safe State - Log Error

- **Preconditions**: Critical error
- **Actions**: Log the error
- **Expected result**: Error recorded
- **Success criteria**: Log created with timestamp and details

##### TEST-ERR-012: Waiting for Operator Action

- **Preconditions**: Safe state applied
- **Actions**: Wait for intervention
- **Expected result**: System waiting
- **Success criteria**: State = "Operator_waiting"

##### TEST-ERR-013: Operator Action - Reset

- **Preconditions**: Waiting for operator
- **Actions**: Operator performs a reset
- **Expected result**: System reset
- **Success criteria**: Back to the initial state

##### TEST-ERR-014: Operator Action - Diagnostic (If time allows)

- **Preconditions**: Waiting for operator
- **Actions**: Operator runs a diagnostic
- **Expected result**: Diagnostic report generated
- **Success criteria**: Report available

##### TEST-ERR-015: Operator Action - Stop

- **Preconditions**: Waiting for operator
- **Actions**: Operator stops the system
- **Expected result**: System stopped cleanly
- **Success criteria**: Full stop without damage

---

### MODULE 6: SORT PARCELS (Package Sorting)

#### Goal

Check the operation of the parcel sorting system.

#### Unit Tests

##### TEST-TRI-001: Receiving Read Info

- **Preconditions**: RFID reading done
- **Actions**: Receive the parcel information
- **Expected result**: Info_read != empty
- **Success criteria**: Data received correctly

##### TEST-TRI-002: Set Direction - Output A

- **Preconditions**: Read info available
- **Actions**: Analyze the data, set direction A
- **Expected result**: Direction = A
- **Success criteria**: Direction correctly set

##### TEST-TRI-003: Set Direction - Output B

- **Preconditions**: Read info available
- **Actions**: Analyze the data, set direction B
- **Expected result**: Direction = B
- **Success criteria**: Direction correctly set

##### TEST-TRI-004: Set Direction - Output C

- **Preconditions**: Read info available
- **Actions**: Analyze the data, set direction C
- **Expected result**: Direction = C
- **Success criteria**: Direction correctly set

##### TEST-TRI-005: Sorter Movement to A

- **Preconditions**: Direction = A
- **Actions**: Move the sorter to output A
- **Expected result**: Sorter in position A
- **Success criteria**: Sorter_position = A

##### TEST-TRI-006: Sorter Movement to B

- **Preconditions**: Direction = B
- **Actions**: Move the sorter to output B
- **Expected result**: Sorter in position B
- **Success criteria**: Sorter_position = B

##### TEST-TRI-007: Sorter Movement to C

- **Preconditions**: Direction = C
- **Actions**: Move the sorter to output C
- **Expected result**: Sorter in position C
- **Success criteria**: Sorter_position = C

##### TEST-TRI-008: Sorted Parcel Validation

- **Preconditions**: Sorter movement completed
- **Actions**: Check the sorting status
- **Expected result**: Parcel_sorted = true
- **Success criteria**: Variable updated

##### TEST-TRI-009: Sorting Time

- **Preconditions**: Read info available
- **Actions**: Measure the full sorting time
- **Expected result**: Time < defined_max_time
- **Success criteria**: Acceptable performance

##### TEST-TRI-010: Sequential Sorting of 5 Parcels

- **Preconditions**: 5 parcels with different directions
- **Actions**: Sort the 5 parcels
- **Expected result**: All sorted correctly
- **Success criteria**: 5/5 parcels in the right outputs

---

## INTEGRATION TESTS

### INTEGRATION 1: Scheduler + Check Connect

#### TEST-INT-001: Full Transition

- **Goal**: Check the communication between the scheduler and check connect
- **Actions**:
  1. Start the scheduler
  2. Trigger check connect
  3. Check the Is_Connect_OK return
- **Expected result**: The scheduler correctly receives the status
- **Success criteria**: Smooth transition without data loss

### INTEGRATION 2: Check Connect + Init Position

#### TEST-INT-002: Data Passing

- **Goal**: Check the passing of the connection information
- **Actions**:
  1. Check connect succeeded
  2. Start init position
  3. Check the use of the connection data
- **Expected result**: Init position uses the right connections
- **Success criteria**: No reconnection needed

### INTEGRATION 3: Init Position + Production

#### TEST-INT-003: Production Start

- **Goal**: Check the start of production after init
- **Actions**:
  1. Init position completed
  2. Start production
  3. Check the use of the loaded parameters
- **Expected result**: Production starts with the right parameters
- **Success criteria**: Speed and positions correct

### INTEGRATION 4: Production + Sort Parcels

#### TEST-INT-004: RFID Data Flow

- **Goal**: Check the passing of the read data to the sorter
- **Actions**:
  1. RFID reading in production
  2. Transmission to the sorter
  3. Sorting done
- **Expected result**: Data transmitted without loss
- **Success criteria**: The sorting matches the data read

### INTEGRATION 5: Error Management + All Modules

#### TEST-INT-005: Error in Check Connect

- **Goal**: Check error handling during check connect
- **Actions**:
  1. Simulate a connection failure
  2. Check the error management intervention
  3. Check the return to the scheduler
- **Expected result**: Error handled, system in a safe state
- **Success criteria**: No crash, consistent state

#### TEST-INT-006: Error in Production

- **Goal**: Check error handling during production
- **Actions**:
  1. Simulate an RFID error
  2. Check that the conveyor belt stops
  3. Check the recovery attempt
- **Expected result**: Production stopped cleanly
- **Success criteria**: No lost parcel

---

## FULL APPLICATION TESTS

### SYSTEM TEST 1: Nominal Full Cycle

#### TEST-SYS-001: Start to Stop

- **Goal**: Test a full cycle of the system
- **Estimated duration**: 10 minutes
- **Preconditions**:
  - System powered off
  - All devices connected
  - 10 parcels prepared with RFID tags
- **Procedure**:
  1. Start the system
  2. Check that all the states are traversed (Waiting → Check → Init → Production)
  3. Place 10 parcels on the belt
  4. Watch the full sorting
  5. Press the stop button
  6. Check the clean stop
- **Expected results**:
  - All the states traversed without error
  - 10 parcels sorted correctly
  - Clean stop of the system
- **Success criteria**: 100% success, time < 10 min

### SYSTEM TEST 2: Performance and Load

#### TEST-SYS-002: Normal Load (50 parcels/hour)

- **Goal**: Check performance under normal load
- **Estimated duration**: 1 hour
- **Actions**: Process 50 parcels in 1 hour
- **Expected results**:
  - 50/50 parcels sorted correctly
  - Average time per parcel < 72 seconds
  - No system error
- **Success criteria**: Success rate > 98%

#### TEST-SYS-003: Maximum Load (100 parcels/hour)

- **Goal**: Check performance under maximum load
- **Estimated duration**: 1 hour
- **Actions**: Process 100 parcels in 1 hour
- **Expected results**:
  - > 95 parcels sorted correctly
  - Average time per parcel < 36 seconds
  - Errors handled correctly
- **Success criteria**: Success rate > 95%

### SYSTEM TEST 3: Robustness and Reliability

#### TEST-SYS-004: 8-Hour Continuous Operation

- **Goal**: Check stability over a long period
- **Estimated duration**: 8 hours
- **Actions**: Run the system continuously for 8 hours
- **Expected results**:
  - No system crash
  - Stable performance
  - Stable memory (no leak)
- **Success criteria**: Operation without interruption

#### TEST-SYS-005: Restart after Critical Error

- **Goal**: Check recovery after a critical error
- **Actions**:
  1. System in production
  2. Simulate a critical error (motor disconnection)
  3. Reconnect the device
  4. Restart the system
- **Expected results**: Successful restart, production resumes
- **Success criteria**: System operational in < 5 min

### SYSTEM TEST 4: Edge Cases and Errors

#### TEST-SYS-006: Parcels without RFID

- **Goal**: Check the handling of faulty parcels
- **Actions**: Introduce 5 parcels without RFID tags among 20 normal parcels
- **Expected results**:
  - 20 normal parcels sorted
  - 5 faulty parcels detected and isolated
  - Production continues
- **Success criteria**: No lost parcel, errors logged

#### TEST-SYS-007: Loss of Dolibarr Connection

- **Goal**: Check the handling of an external connection loss
- **Actions**:
  1. System in production
  2. Cut the Dolibarr connection
  3. Observe the system's reaction
- **Expected results**:
  - Error detected
  - Automatic reconnection attempt
  - If it fails: safe state
- **Success criteria**: No data loss

#### TEST-SYS-008: Sorter Mechanical Blockage

- **Goal**: Check the detection of a mechanical blockage
- **Actions**:
  1. Mechanically block the sorter
  2. Try to sort a parcel
- **Expected results**:
  - Timeout detected
  - Critical error raised
  - Conveyor belt stopped
- **Success criteria**: Safety ensured, no damage

### SYSTEM TEST 5: Interface and Monitoring

#### TEST-SYS-009: Logs and Traceability

- **Goal**: Check the quality of the logs
- **Actions**: Run a full cycle and analyze the logs
- **Expected results**:
  - All the events logged
  - Correct timestamps
  - Appropriate log level
- **Success criteria**: Logs usable for diagnosis

#### TEST-SYS-010: Emergency Stop Button

- **Goal**: Check the emergency stop
- **Actions**: Press BP_Stop during production
- **Expected results**:
  - Immediate stop (< 2 seconds)
  - Conveyor belt and sorter stopped
  - State saved
- **Success criteria**: Immediate and safe stop

---

## INTERVENTION PROCEDURES

### INTERVENTION 1: Connection Error

#### Symptoms

- Is_Connect_OK = false
- Connection error message

#### Diagnosis

1. Check the network connection
2. Ping the devices
3. Check the connection logs

#### Corrective Actions

1. **If Dolibarr**: Check the server, credentials, network
2. **If Motor**: Check the wiring, power supply, driver
3. **If Sorter**: Check the wiring, power supply, position
4. **If RFID**: Check the antenna, wiring, configuration

#### Restart

1. Fix the identified problem
2. Run Check Connect again
3. Check that Is_Connect_OK = true

### INTERVENTION 2: Initialization Error

#### Symptoms

- Is_Prod_OK = false
- Error during Init Position

#### Diagnosis

1. Check the parameter loading
2. Check the initial sorter position
3. Check the initialization logs

#### Corrective Actions

1. **Parameters not loaded**: Check the config file, reload
2. **Sorter blocked**: Check it mechanically, unblock, reset
3. **Timeout**: Increase the timeout if needed, check the speed

#### Restart

1. Fix the problem
2. Run Init Position again
3. Check that Is_Prod_OK = true

### INTERVENTION 3: Error in Production

#### Symptoms

- Production stopped
- RFID or sorting error

#### Diagnosis

1. Identify the error type (logs)
2. Check the state of the devices
3. Check the parcels in progress

#### Corrective Actions

1. **Unreadable RFID**: Remove the parcel, clean the reader, test
2. **Sorter blocked**: Unblock it, check the mechanics, reset
3. **Conveyor belt stopped**: Check the motor, restart

#### Restart

1. Remove the problematic parcel if needed
2. Reset the affected module
3. Restart production

### INTERVENTION 4: Critical Error

#### Symptoms

- System in safe state
- Waiting for operator action

#### Diagnosis

1. Check the critical error logs
2. Identify the root cause
3. Assess the potential damage

#### Corrective Actions

1. **Reset**: If the error is temporary, reset the system
2. **Diagnostic**: If the cause is unknown, run a full diagnostic
3. **Stop**: If the problem is major, stop and call maintenance

#### Restart

1. Fix the identified problem
2. Run a verification test
3. Restart from Waiting

---

## TEST GENERATION AND EXECUTION

### Automated Test Structure

#### File Organization

```
tests/
├── unit/
│   ├── test_scheduler.py
│   ├── test_check_connect.py
│   ├── test_init_position.py
│   ├── test_production.py
│   ├── test_error_management.py
│   └── test_sort_parcels.py
├── integration/
│   ├── test_scheduler_check.py
│   ├── test_check_init.py
│   ├── test_init_production.py
│   ├── test_production_sorter.py
│   └── test_error_integration.py
├── system/
│   ├── test_full_cycle.py
│   ├── test_performance.py
│   ├── test_robustness.py
│   └── test_edge_cases.py
├── fixtures/
│   ├── mock_dolibarr.py
│   ├── mock_motor.py
│   ├── mock_sorter.py
│   └── mock_rfid.py
└── utils/
    ├── test_helpers.py
    └── assertions.py
```

### Recommended Test Framework

#### For Python

- **pytest**: Main framework
- **pytest-mock**: For the mocks
- **pytest-timeout**: For the timeouts
- **pytest-cov**: For the code coverage

#### For C/C++

- **Google Test**: Main framework
- **Google Mock**: For the mocks
- **CTest**: CMake integration

### Execution Commands

#### Unit Tests

```bash
# All unit tests
pytest tests/unit/ -v

# Specific module
pytest tests/unit/test_scheduler.py -v

# Specific test
pytest tests/unit/test_scheduler.py::test_initial_state -v
```

#### Integration Tests

```bash
# All integration tests
pytest tests/integration/ -v

# With a 60s timeout per test
pytest tests/integration/ -v --timeout=60
```

#### System Tests

```bash
# System tests (longer)
pytest tests/system/ -v --timeout=600

# Performance test only
pytest tests/system/test_performance.py -v
```

#### Code Coverage

```bash
# Generate the coverage report
pytest tests/ --cov=src --cov-report=html

# View the report
open htmlcov/index.html
```

### Unit Test Template

```python
import pytest
from unittest.mock import Mock, patch
from src.scheduler import Scheduler

class TestScheduler:
    """Unit tests for the Scheduler module"""

    @pytest.fixture
    def scheduler(self):
        """Fixture to create a scheduler instance"""
        return Scheduler()

    def test_initial_state(self, scheduler):
        """TEST-ORD-001: Initial State"""
        # Arrange
        # (already done by the fixture)

        # Act
        state = scheduler.get_state()

        # Assert
        assert state == "Waiting", "The initial state must be Waiting"

    def test_transition_waiting_check_connect(self, scheduler):
        """TEST-ORD-002: Transition Waiting → Check Connect"""
        # Arrange
        assert scheduler.get_state() == "Waiting"

        # Act
        scheduler.trigger_check_connect()

        # Assert
        assert scheduler.get_state() == "Check Connect"

    def test_condition_is_connection_true(self, scheduler):
        """TEST-ORD-003: Condition Is_Connection = true"""
        # Arrange
        scheduler.set_state("Check Connect")
        scheduler.set_is_connection(True)

        # Act
        scheduler.evaluate_transition()

        # Assert
        assert scheduler.get_state() == "Init Positions"
```

### Integration Test Template

```python
import pytest
from src.scheduler import Scheduler
from src.check_connect import CheckConnect

class TestIntegrationSchedulerCheck:
    """Integration tests Scheduler + Check Connect"""

    @pytest.fixture
    def system(self):
        """Fixture to create the full system"""
        scheduler = Scheduler()
        check_connect = CheckConnect()
        scheduler.set_check_connect(check_connect)
        return scheduler, check_connect

    def test_full_transition(self, system):
        """TEST-INT-001: Full Transition"""
        # Arrange
        scheduler, check_connect = system

        # Act
        scheduler.start()
        scheduler.trigger_check_connect()
        result = check_connect.check_connections()

        # Assert
        assert result.is_connect_ok == True
        assert scheduler.get_state() == "Init Positions"
```

### System Test Template

```python
import pytest
import time
from src.full_system import IOTSystem

class TestFullSystem:
    """System tests for the full application"""

    @pytest.fixture
    def system(self):
        """Fixture to create the full system"""
        system = IOTSystem()
        system.initialize()
        yield system
        system.stop()

    @pytest.mark.slow
    def test_nominal_full_cycle(self, system):
        """TEST-SYS-001: Nominal Full Cycle"""
        # Arrange
        parcels = [self.create_test_parcel(i) for i in range(10)]

        # Act
        start = time.time()
        system.start()

        for parcel in parcels:
            system.add_parcel(parcel)

        system.wait_for_processing_end()
        duration = time.time() - start

        # Assert
        assert system.get_sorted_parcel_count() == 10
        assert duration < 600  # 10 minutes max
        assert system.get_error_count() == 0

    def create_test_parcel(self, id):
        """Create a test parcel"""
        return {
            'id': id,
            'rfid': f'RFID_{id:04d}',
            'destination': ['A', 'B', 'C'][id % 3]
        }
```

### Continuous Execution (CI/CD)

#### GitHub Actions Configuration

```yaml
name: IOT Tests

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

### Test Report

#### Report Format

```
TEST REPORT - [DATE]
====================

Module: [MODULE_NAME]
Version: [VERSION]
Tester: [NAME]

SUMMARY
-------
Tests executed: XX
Tests passed: XX
Tests failed: XX
Success rate: XX%

DETAILS
-------
[TEST-XXX-001] Test name: ✓ PASS
[TEST-XXX-002] Test name: ✗ FAIL
  Error: [Description]

CODE COVERAGE
-------------
Module A: XX%
Module B: XX%
Global: XX%

RECOMMENDATIONS
---------------
- [Recommendation 1]
- [Recommendation 2]
```

---

## APPENDICES

### Pre-Test Checklist

- [ ] All the devices are connected
- [ ] Stable power supply
- [ ] Network operational
- [ ] Dolibarr reachable
- [ ] Test parcels prepared
- [ ] Logs enabled
- [ ] Sufficient disk space

### Post-Test Checklist

- [ ] Logs saved
- [ ] Results documented
- [ ] Anomalies reported
- [ ] Devices stopped cleanly
- [ ] Test area cleaned up

### Emergency Contacts

- **Technical Support**: [CONTACT]
- **Maintenance**: [CONTACT]
- **Project Manager**: [CONTACT]

### Glossary

- **BP_Stop**: Push button stop
- **RFID**: Radio Frequency Identification
- **Is_Connect_OK**: Boolean variable indicating the connections are OK
- **Is_Prod_OK**: Boolean variable indicating production is OK
- **Dolibarr**: ERP/CRM system

---

**Document created on**: [DATE]
**Version**: 1.0
**Author**: IoT Test Team
**Status**: Draft
