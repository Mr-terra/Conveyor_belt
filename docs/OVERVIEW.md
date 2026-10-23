# System Overview

A summary of how the conveyor works, module by module. It describes the current firmware. The
diagrams are in [docs/uml/](uml/README.md), the operator guide in [SYSTEM_USAGE.md](SYSTEM_USAGE.md).

## 1. What the system does

A parcel is put on a belt. The RFID reader reads its tag. The tag UID is looked up in Dolibarr,
which gives the parcel's warehouse. A servo moves a diverter to the bin for that warehouse, and a
stock movement is recorded in Dolibarr.

## 2. Modules

| Module | File(s) | Job |
|--------|---------|-----|
| Scheduler | `src/scheduler/` | Runs the system states: WAITING, CHECK_CONNECT, INIT_POSITIONS, PRODUCTION, ERROR |
| CheckConnect | `src/check_connect/` | Connection checks before production |
| InitPosition | `src/init_position/` | Position initialization (placeholder) |
| Conveyor | `src/conveyor/` | Reads the GRBL motor status, drives the belt |
| SorterController | `src/sorter/main_sorter.cpp` | One sorting cycle: read, route, move the diverter |
| RFIDReader | `src/sorter/rfid_reader/` | Reads tag UIDs, with a cooldown |
| Addresser | `src/sorter/addresser/` | UID → Dolibarr product → warehouse → bin |
| ServoSorter | `src/sorter/servo_sorter/` | Moves the diverter to a bin angle |
| DolibarrAPI | `src/api/` | REST calls to Dolibarr |
| StateManager | `src/state_manager/` | Watches the module states, applies the rules |
| Error | `src/error.cpp` | Error levels and the critical latch |
| main | `src/main.cpp` | `setup()`, `loop()`, buttons |

The logic only talks to the hardware through the interfaces in `include/hal/`. The ESP32
implementations are in `src/hal/firmware/`.

## 3. Scheduler

States: `WAITING`, `CHECK_CONNECT`, `INIT_POSITIONS`, `PRODUCTION`, `ERROR`.

| From | To | When |
|------|----|------|
| WAITING | CHECK_CONNECT | Button A |
| CHECK_CONNECT | INIT_POSITIONS | All checks pass |
| CHECK_CONNECT | WAITING | A check fails |
| INIT_POSITIONS | PRODUCTION | Initialization passes |
| INIT_POSITIONS | WAITING | Initialization fails |
| PRODUCTION | WAITING | Button B (stop) |
| PRODUCTION | ERROR | Conveyor alarm (see section 9) |
| ERROR | WAITING | Automatic recovery: no latched critical error, and connections OK |
| ERROR | CHECK_CONNECT | Button A: clears the latched critical error, then starts the checks |

Each pass of `loop()` runs the step of the current state. In `PRODUCTION`, the conveyor moves
forward and the sorter runs one cycle.

## 4. Check Connect

Run before production, after Button A.

| Check | What it does | Status |
|-------|--------------|--------|
| GRBL motor | I2C probe at `0x70`, status must answer, module must not be locked | Active |
| Sorter | RFID probe at `0x28` and servo board probe at `0x38` | Active |
| Dolibarr | HTTP check of the API, must answer 200 | Active |

If any check fails, Start goes back to `WAITING`. The same Dolibarr check also runs once at boot from
`loop()` and logs `[INFO] Main: Dolibarr OK` or `[ERROR] Main: Dolibarr check failed`. A parcel whose lookup fails goes to the error
bin (section 7).

## 5. Init Position

Two steps: load motor parameters, then initialize the sorter position. Both only log and return
`true`. The step is a placeholder, so the state machine can run end to end. It does not talk to the
hardware yet.

## 6. Production

Each pass of `loop()` in `PRODUCTION`:

1. If the conveyor is in `ALARM`, go to `WAITING` (guard in `executeCurrentState()`).
2. Otherwise move the conveyor forward.
3. Run one sorter cycle (section 7).

## 7. Sorting one parcel

1. **Read.** `RFIDReader::readUid()` returns the UID as `XX:XX:XX:XX`. A card read within 1.5 s of the
   last accepted read is halted and ignored. No card gives an empty UID, and the cycle ends.
2. **Look up.** `Addresser` calls `GET /products?sqlfilters=(barcode:=:'<uid>')`. The UID colons are
   removed first. An empty response, invalid JSON, or a non-array answer gives `NONE`.
3. **Find the warehouse.** The product's `default_target_warehouse_id` is read, under either
   `options_options_default_target_warehouse_id` or `options_default_target_warehouse_id`.
4. **Record the movement.** For a known warehouse (1, 2 or 3), `POST /stockmovements` with quantity 1.
   It must return HTTP 200 or 201.
5. **Route.**

| Case | Bin | Servo angle |
|------|-----|-------------|
| Warehouse 1, movement accepted | BIN1 | 0° |
| Warehouse 2, movement accepted | BIN2 | 30° |
| Warehouse 3, movement accepted | BIN3 | 60° |
| Unknown card, failed lookup, unknown warehouse | ERROR BIN = BIN1 | 0° |
| Warehouse known, movement rejected | ERROR BIN = BIN1 | 0° |

The error bin is shared with warehouse 1. The serial log is the only way to tell those parcels apart.

## 8. Conveyor and GRBL monitoring

- `monitorHardware()` reads the GRBL status every 100 ms and sets the conveyor state.
- Status letters: `I` → STOPPED, `A` → ALARM, `R`, `J`, `H` → RUNNING.
- **Silent module watchdog:** if the module gives no answer, the code falls back to the lock and idle
  queries. After 10 missed reads in a row (about 1 s), the state becomes ALARM and one critical error is
  raised. Any answer resets the count.

## 9. Error management

| Level | Meaning |
|-------|---------|
| INFO | Normal change, for the log (e.g. motor started) |
| WARNING | Non-critical (e.g. recovered from alarm, state stuck for 5 s) |
| CRITICAL | During production, stops the belt (back to WAITING). Stays latched until Button A |
| FATAL | The scheduler goes to ERROR. Stays latched until Button A |

When a conveyor state changes, `StateManager` runs the matching rules:

- A change to ALARM matches two rules: CRITICAL (from RUNNING or STOPPED) and FATAL (from any state).
  During production the final state is **ERROR**, not WAITING. The belt is stopped either way.
- Automatic recovery never clears a latched critical error. It is cleared only when the operator
  presses **A** from ERROR or WAITING.
- The stuck warning (WARNING) fires after 5 s in a transitional state. Steady states (WAITING,
  STOPPED, RUNNING, PRODUCTION) are not reported.

## 10. Dolibarr interface

| Call | Used for |
|------|----------|
| `GET /products?sqlfilters=(barcode:=:'…')` | Find the product and its warehouse from the tag |
| `POST /stockmovements` (`product_id`, `warehouse_id`, `qty`) | Record one parcel leaving the warehouse |

Requests use the `DOLAPIKEY` header. Each request has a 5 s timeout. The stock-movement call
returns true only for HTTP 200 or 201.

## 11. Known limits

1. **Servo parks at 30° at boot,** the same angle as BIN2. This has not been checked on a machine.
2. **The error bin is shared with warehouse 1.** Parcels in error and parcels for warehouse 1 end up
   in the same bin.
3. **No hardware emergency stop.** Button B stops in software only.
4. **Init Position is a placeholder.**
5. **Not run on the hardware.** See [TEST_PROTOCOL.md](TEST_PROTOCOL.md) section 3.
