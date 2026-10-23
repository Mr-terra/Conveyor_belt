# State Flow and Error Handling

This document explains how the firmware knows the state of the hardware, how a change becomes an
error or a state transition, and where each step lives in the code. It describes the current
code; for the operator's view see [SYSTEM_USAGE.md](SYSTEM_USAGE.md).

## 1. Principles

- **The hardware is the source of truth.** The conveyor state is read from the GRBL module, not
  set by the software. A command such as "start" does not mean the belt moved: the next read shows
  whether it did.
- **Detection by comparison.** The `StateManager` remembers the last state of each module. When a
  read differs, it reports a change.
- **Declarative rules.** A rule says "when module X goes from state A to state B, raise an error of
  level L". The rules do not contain control flow.
- **Errors are observations.** `Error::triggerError` logs the error and calls the callbacks. The
  reaction (stop, go to ERROR) is decided in `StateManager::handleCriticalStateChange`.

## 2. Components

| Component | File | Role |
|-----------|------|------|
| `Conveyor` | `src/conveyor/conveyor.cpp` | Reads the GRBL status every 100 ms and maps it to `STOPPED`, `RUNNING` or `ALARM` |
| `StateManager` | `src/state_manager/state_manager.cpp` | Watches registered state variables, applies the rules, warns about stuck states |
| `Scheduler` | `src/scheduler/scheduler.cpp` | Runs the sequence `WAITING → CHECK_CONNECT → INIT_POSITIONS → PRODUCTION`, and `ERROR` |
| `Error` | `src/error.cpp` | Levels `INFO`, `WARNING`, `CRITICAL`, `FATAL`; the critical latch |
| `main.cpp` | `src/main.cpp` | `setup()` and `loop()`, button handling |

## 3. Start-up

`setup()` in `src/main.cpp`, simplified:

1. `M5.begin()`, I2C on pins 21 and 22, then `bindFirmwareHardware()` connects the hardware
   adapters to the logic.
2. `conveyor.initialize(stateManager)`: reads the first status, then registers the conveyor state
   variable with the `StateManager` and adds the conveyor rules.
3. `sorterController.initialize(stateManager)`, `scheduler.initialize()`, `initPosition.initialize()`.
4. Wi-Fi connects (15 s timeout). The Dolibarr check runs once Wi-Fi is up.
5. The scheduler gets its dependencies and the `StateManager` gets a reference to the scheduler.
6. The scheduler is set to `WAITING`. Nothing moves until **A** is pressed.

Registration in step 2 is what makes state changes visible later: the `StateManager` stores a
pointer to `_currentState` and a function that turns it into text.

## 4. The loop

`loop()` runs these steps in order, on every pass (about every 100 ms):

1. `conveyor.monitorHardware()` reads the GRBL status, at most once every 100 ms, and updates
   `_currentState`. This is the only place the conveyor state changes.
2. `stateManager.monitorAllStates()` compares each registered state with its last value. On a
   change, it runs the matching rules.
3. Button handling (A, B, C).
4. `scheduler.executeCurrentState()` runs the current scheduler step.

Because the hardware read comes first, a change is always seen in the same pass as the code that
reacts to it.

## 5. Conveyor states and rules

The GRBL status letter is mapped as follows (`readHardwareState()`):

| GRBL status | Conveyor state |
|-------------|----------------|
| `I` (idle) | `STOPPED` |
| `A` (alarm) | `ALARM` |
| `R`, `J`, `H` and others | `RUNNING` |
| no answer | see the watchdog below |

When the module gives no answer, the code falls back to the lock and idle queries. After **10
consecutive** missed reads (1 s), the state is `ALARM` and a critical error is raised once,
`GRBL module not responding`.

The rules registered by the conveyor (`setupStateRules`):

| From | To | Level |
|------|----|-------|
| `STOPPED` | `RUNNING` | INFO |
| `RUNNING` | `STOPPED` | INFO |
| `RUNNING` | `ALARM` | CRITICAL |
| `STOPPED` | `ALARM` | CRITICAL |
| any (`*`) | `ALARM` | FATAL |
| `ALARM` | `STOPPED` | WARNING |

### What happens on an alarm

A transition to `ALARM` matches **two** rules: the CRITICAL rule for the previous state and the
FATAL rule (`* → ALARM`). `handleCriticalStateChange` runs for each match, in order:

1. CRITICAL: if the scheduler is in `PRODUCTION`, it goes to `WAITING`.
2. FATAL: the scheduler goes to `ERROR`.

So in the current firmware an alarm during production ends in **`ERROR`**, not `WAITING`. The belt
is stopped and **A** is needed to restart, and the critical latch is kept. The host test
`ConveyorAlarmDuringProductionEndsInError` checks this with the real rules.

## 6. Scheduler and recovery

- `WAITING`: nothing is driven. **A** starts `CHECK_CONNECT`.
- `CHECK_CONNECT`: checks Dolibarr, the GRBL module, the RFID reader and the servo board. On
  success it goes to `INIT_POSITIONS`; on failure, back to `WAITING`.
- `INIT_POSITIONS`: placeholder steps; success moves to `PRODUCTION`.
- `PRODUCTION`: the conveyor runs and the sorter handles each card. An alarm stops it (section 5).
- `ERROR`: each pass tries to recover. Recovery is refused while a critical error is latched, and
  when the connections are not all healthy. A latched critical error is cleared only by the operator
  pressing **A** from `ERROR` or `WAITING` (`src/main.cpp`), which then runs `CHECK_CONNECT`.

The latch is intentional: an automatic reconnection must not restart a belt that stopped on a
critical error.

## 7. Stuck-state warnings

`monitorAllStates()` also watches for a state that stays the same for more than **5 seconds**
(`stuckStateTimeout`). Only transitional states are reported. Steady operating states (`WAITING`,
`STOPPED`, `INITIAL`, `IDLE`, `RUNNING`, `PRODUCTION`) are expected to last, so they are ignored.
The warning is `ERROR_WARNING` and is given once per stay in the state.

## 8. Open points

- **No hardware emergency stop.** The software stop (button **B**) cuts the command, not the power.
- **`InitPosition` is a placeholder** and does not check the hardware.
