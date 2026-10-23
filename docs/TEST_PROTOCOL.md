# Test Protocol

This document describes what is tested, how to run it, and what must be checked on the
hardware. The earlier school protocol (about 1,100 lines, with a Python test plan that was never
implemented) is still in the git history.

## 1. Automated host tests

The firmware logic runs on the PC. The tests use googletest and gMock, a fake Arduino core
(`test/host/include/Arduino.h`) and mocks for every interface in `include/hal/`
(`test/host/fakes/hardware_mocks.h`).

```bash
cmake -S . -B build -DBUILD_TESTS=ON
cmake --build build
ctest --test-dir build --output-on-failure
```

The current suite has **99 tests** in 11 files, all passing. CI runs the same commands on every
push to `main`.

### Coverage by module

| Module | File | Tests | What is checked |
|--------|------|------:|-----------------|
| Error | `error_test.cpp` | 7 | Callbacks, levels, critical latch, log format |
| Conveyor | `conveyor_test.cpp` | 21 | GRBL status mapping, lock/idle fallback, silent-module watchdog (ALARM after 10 missed reads), start/stop/move commands, speed clamping, connection checks |
| State manager | `state_manager_test.cpp` | 10 | Rules on transitions, stuck-state warnings, conveyor alarm during production (ends in ERROR, using the real rules), fatal alarm (ERROR) |
| Scheduler | `scheduler_test.cpp` | 14 | Full sequence WAITING → CHECK_CONNECT → INIT_POSITIONS → PRODUCTION, failure paths, ERROR recovery, critical latch blocks recovery until cleared |
| Check connect | `check_connect_test.cpp` | 10 | Each device check, Dolibarr reachability with and without Wi-Fi, a Dolibarr error fails the checks, unconfigured Dolibarr skipped |
| Init position | `init_position_test.cpp` | 1 | Placeholder steps succeed together |
| Dolibarr API | `dolibarr_api_test.cpp` | 6 | Request URLs and API key header, transport failure returns empty body, stock movement JSON, connection check on 200 / 401 |
| Addresser | `addresser_test.cpp` | 10 | Warehouse → bin routing, `warehouse_id` and `options_…` keys, unknown card and failed stock movement go to the error bin, malformed or empty responses |
| RFID reader | `rfid_reader_test.cpp` | 7 | UID format, card halted after each read, 1.5 s cooldown, I2C address `0x28` |
| Servo sorter | `servo_sorter_test.cpp` | 6 | Angles 0° / 30° / 60°, unknown bin goes to the error bin (0°), GoPlus address `0x38` |
| Sorter controller | `sorter_controller_test.cpp` | 7 | Device checks, full sort cycle, unknown card goes to the error bin |

### Not covered by the host tests

- Real I2C timing and the GRBL, MFRC522 and GoPlus2 chips (only the HAL adapters in
  `src/hal/firmware/` touch them, and they are not compiled on the PC).
- Wi-Fi association, HTTP over the network, the real Dolibarr server.
- Physical servo position and parcel movement.
- Button handling in `main.cpp`.

Those are checked by the hardware protocol below.

## 2. Build check

The firmware must compile for the board. CI does this on every push to `main`:

```bash
pio run -e m5stack-grey
```

A compile error that only shows up in the firmware (for example a library API change) is not
caught by the host tests. Always run this before flashing.

## 3. Hardware acceptance protocol

Run this after each firmware change, before the system is used. **These steps have not been run** on the hardware: the project is no longer connected to a board. The table is kept as a reference for whoever tests it next. Use the serial monitor at
115200 baud (`pio device monitor -e m5stack-grey`).

Prerequisites: the `.env` file is filled in, the firmware is flashed, the GRBL module is at
`0x70`, the RFID reader at `0x28` and the GoPlus2 board at `0x38`, and Dolibarr is reachable.

| # | Step | Expected result | Result |
|---|------|-----------------|:------:|
| H1 | Power on | Serial log shows `WiFi OK` and `Dolibarr OK`; screen shows `WAITING` | |
| H2 | Power on with Dolibarr stopped | `Dolibarr FAIL` is logged; the board still runs, and Start stops at the connection check and stays in `WAITING` | |
| H3 | Press **A** | `CHECK_CONNECT` then `INIT_POSITIONS` then `PRODUCTION`; the conveyor moves forward | |
| H4 | Present a known card | Log shows the UID and the warehouse ID, no `[ERR]` line, and the servo moves to the bin for that warehouse | |
| H5 | Verify the stock movement in Dolibarr | A movement exists for the product, warehouse and quantity 1 | |
| H6 | Present an unknown card | `Unknown UID`; the servo moves to 0° (BIN1, the error bin) | |
| H7 | Block the Dolibarr server during a card read | The request fails after about 5 s; the board does not freeze; the parcel still goes to the error bin | |
| H8 | Unplug the GRBL module during production | After about 1 s the conveyor is in `ALARM`, production stops, and a critical error is logged once | |
| H9 | Plug the GRBL module back in, press **B**, then **A** | **B** stops the conveyor; **A** clears the latched error and runs the checks again | |
| H10 | Press **C** | The screen shows the current mode and conveyor state for 2 seconds | |
| H11 | Physically check the servo angles | Each bin's diverter lines up with its bin at 0°, 30° and 60° | |

The servo angles in H11 are not validated yet. The values come from the code (`angleFor()` in
`src/sorter/servo_sorter/servo_sorter.cpp`) and must be checked on the real mechanism.

## 4. Record of results

| Date | Firmware commit | Automated tests | Hardware steps passed | Notes |
|------|-----------------|-----------------|-----------------------|-------|
| | | 99 / 99 (local build, GCC on Linux) | 0 / 11 (not run, no board access) | Hardware steps not run |
