# User Guide: RFID Parcel Sorting Conveyor

This guide is for the people who run the conveyor: how to start it, what the screen and the
buttons do, what the messages mean, and how to recover from a fault.

For building and flashing the firmware, see the [README](../README.md).

## Contents

1. [How the system works](#how-the-system-works)
2. [Before the first start](#before-the-first-start)
3. [Daily use](#daily-use)
4. [Sorting bins](#sorting-bins)
5. [Errors and recovery](#errors-and-recovery)
6. [Messages in the serial monitor](#messages-in-the-serial-monitor)
7. [Troubleshooting](#troubleshooting)

---

## How the system works

A GRBL motor module drives the belt. An RFID reader reads the tag of each parcel. The tag
UID is looked up in Dolibarr, which returns the parcel's destination warehouse. A servo moves
the diverter to the bin for that warehouse, and a stock movement is recorded in Dolibarr.

The system runs a fixed sequence:

```
WAITING ──(A)──▶ CHECK_CONNECT ──ok──▶ INIT_POSITIONS ──ok──▶ PRODUCTION
   ▲                  │                      │                    │
   └──────── fail ────┴──────── fail ────────┘                    │
   ▲                                                              │
   └──────────────(B: stop, back to WAITING)─────────────────────┘
   (a conveyor alarm during production goes to ERROR)
```

- **WAITING**: nothing moves. The system waits for Start.
- **CHECK_CONNECT**: Dolibarr, the GRBL module, the RFID reader and the servo board are tested.
- **INIT_POSITIONS**: placeholder step, it does not move any hardware yet.
- **PRODUCTION**: the conveyor runs and parcels are sorted.
- **ERROR**: a serious fault stopped the system. See [Errors and recovery](#errors-and-recovery).

---

## Before the first start

1. Power the GRBL module, the RFID reader and the servo board, and check their I2C wiring
   (SDA on GPIO 21, SCL on GPIO 22).
2. Power the M5Stack. The screen shows the mode (`WAITING`) and the conveyor state.
3. Watch the serial monitor (115200 baud). The start-up log should show:
   - `✅ WiFi OK, IP=...`, once the board has joined the Wi-Fi network.
   - `[INFO] Main: Dolibarr OK`, once the Dolibarr API has answered. If it shows `[ERROR] Main: Dolibarr check failed`, the
     board keeps running, but Start stops at the connection check until the API answers. Fix the URL or the key, then reflash.

The Wi-Fi network, the Dolibarr URL and the API key come from the `.env` file at build time.
They are not editable on the device. To change them, edit `.env`, rebuild and reflash.

---

## Daily use

### Buttons

| Button | Action |
|--------|--------|
| **A**  | Start: runs the checks, then the position step, then production. From `ERROR`, it first clears a latched critical error. |
| **B**  | Emergency stop in software: the conveyor stops and the system returns to `WAITING`. |
| **C**  | Shows the current mode and conveyor state on the screen for 2 seconds. |

### Normal cycle

1. Check that the belt is clear and the screen shows `WAITING`.
2. Press **A**. Wait for the screen to show the production mode.
3. Place the parcels on the belt. Each tag is read once; a second read of the same tag within
   about 1.5 seconds is ignored.
4. Press **B** to stop. The conveyor stops and the system returns to `WAITING`.

> **Note:** **B** is a software stop. It does not cut the motor power. If the belt must stop
> immediately, cut the power at the supply, not only in software.

---

## Sorting bins

| Destination | Bin | Diverter angle |
|-------------|-----|----------------|
| Warehouse 1 | BIN1 | 0° |
| Warehouse 2 | BIN2 | 30° |
| Warehouse 3 | BIN3 | 60° |
| **Error bin** | **BIN1** | 0° |

The error bin is shared with warehouse 1. A parcel goes to the error bin when:

- its tag is not known in Dolibarr, or
- the Dolibarr lookup fails (no answer, or a timeout after 5 seconds), or
- the stock movement is refused by Dolibarr.

Because of the shared bin, check the serial log to tell an error parcel from a warehouse 1
parcel (`unknown card` or `[ERROR] Addresser` lines).

The warehouse-to-bin table is in `src/sorter/addresser/addresser.cpp`. The angles are in
`src/sorter/servo_sorter/servo_sorter.cpp`.

---

## Errors and recovery

Errors have four levels: `INFO`, `WARNING`, `CRITICAL` and `FATAL`.

| Situation | Effect |
|-----------|--------|
| Conveyor alarm (any state) | Production stops and the system goes to `ERROR`. Press **A** to restart. |
| Conveyor silent for about 1 second (no status from the GRBL module) | Treated as an alarm. A **critical** error is raised once. |
| Critical error | The error stays latched after the system stops. |

A latched critical error is never cleared automatically. To restart:

1. Press **B** if the system is still running.
2. Find and fix the cause (see [Troubleshooting](#troubleshooting)).
3. Press **A**. This acknowledges the error, clears the latch, and runs the checks again. If
   the fault is still there, the checks fail and the system stays in `WAITING`.

If the M5Stack stops responding completely, cut the power and start again.

---

## Messages in the serial monitor

Every state change, hardware change and error is logged. The messages below are the ones you
are most likely to see.

Each line starts with its level (`[INFO]`, `[WARNING]` or `[ERROR]`), then the module name.

| Message | Meaning |
|---------|---------|
| `[ERROR] Main: WiFi connect timeout` | The board did not join the Wi-Fi network within 15 seconds. Check the network and the `.env` values. |
| `[ERROR] Main: Dolibarr check failed` | The Dolibarr API did not answer with a success status. Check the URL, the key and the server. |
| `[ERROR] Conveyor: GRBL module not detected at 0x70` | The GRBL module does not answer on I2C. Check its power and wiring. |
| `[ERROR] Conveyor: GRBL module is locked, unlock it before use` | The module is locked. It must be unlocked before use. |
| `[ERROR] RFID: reader not detected at 0x28` | The RFID reader does not answer. |
| `[ERROR] Servo: GoPlus2 not detected at 0x38` | The servo board does not answer. Check its power. |
| `[WARNING] Sorter: unknown card <UID>` | The tag is not known in Dolibarr. The parcel goes to the error bin. |
| `[ERROR] Addresser: empty Dolibarr response` | The Dolibarr lookup failed. The parcel goes to the error bin. |
| `[ERROR] Addresser: stock movement rejected, parcel sent to the error bin` | The parcel was sent to the error bin and the stock is not updated. Check the movement in Dolibarr. |
| `[timestamp] CRITICAL [Conveyor]: GRBL module not responding` | The conveyor has been silent for 1 second. See [Errors and recovery](#errors-and-recovery). |

---

## Troubleshooting

### The conveyor does not move

1. Check the GRBL module is powered and at I2C address `0x70`.
2. Press **C** to read the conveyor state. If it shows `ALARM`, press **B** then **A**.
3. Check the motor power supply.

### Parcels go to the wrong bin

1. Check the servo angles on the mechanism (0°, 30°, 60°).
2. Check the parcel's warehouse in Dolibarr. The bin follows the warehouse, not the tag.
3. Remember that the error bin is BIN1, the same as warehouse 1.

### Tags are not read

1. Check the RFID reader is at I2C address `0x28`.
2. Check the distance between the tag and the antenna.
3. Try a second tag. A tag read twice within 1.5 seconds is ignored on purpose.

### Dolibarr is slow or unreachable

A lookup waits at most 5 seconds. A longer wait is not expected; check the network and the
server. The board keeps running and sends affected parcels to the error bin.

### Wi-Fi does not connect

1. Check the SSID and password in `.env`, then rebuild and reflash.
2. Check the Wi-Fi network is 2.4 GHz (the ESP32 does not join 5 GHz networks).

### Reading the logs

Connect the serial monitor at 115200 baud:

```bash
pio device monitor -e m5stack-grey
```

The API key and the Wi-Fi password are not printed.
