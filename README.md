# Conveyor System

[![CI](https://github.com/Mr-terra/Conveyor_belt/actions/workflows/ci.yml/badge.svg)](https://github.com/Mr-terra/Conveyor_belt/actions/workflows/ci.yml)

Firmware for an industrial parcel-sorting conveyor, built for an M5Stack Grey (ESP32).
A GRBL motor module drives the belt, an MFRC522 RFID reader identifies each parcel, a
servo driver (GoPlus2) moves the diverter, and each parcel's destination warehouse is
looked up in Dolibarr over HTTP.

Developed as part of the Epitech MSc Pro IoT project (T-IOT-911).

## Features

- **Scheduler state machine**: `WAITING → CHECK_CONNECT → INIT_POSITIONS → PRODUCTION`, with an `ERROR` state that recovers automatically once the conditions are back to normal.
- **Hardware-driven state**: the conveyor state is read from the GRBL module on every cycle. The software never assumes that a command worked.
- **Error management**: errors are classified (`INFO`, `WARNING`, `CRITICAL`, `FATAL`). A critical error stops the system and stays latched until the operator presses Start (see [Usage](#usage)).
- **Silent module watchdog**: if the GRBL module gives no status for 10 consecutive reads (1 s), the conveyor is treated as in alarm and a critical error is raised.
- **Connection checks**: the Dolibarr API, the GRBL motor, the RFID reader and the servo board are probed before production starts.
- **Sorting flow**: RFID read → Dolibarr product lookup → stock movement → servo moved to the matching bin. Parcels that cannot be routed go to the error bin.
- **Timeouts**: HTTP requests to Dolibarr give up after 5 seconds, so a silent server cannot freeze the controls.
- **Host unit tests**: the firmware logic runs on a PC with googletest and gMock, without an ESP32.

## Architecture

The logic in `src/` depends only on interfaces in `include/hal/` (hardware abstraction layer).
Only `src/hal/firmware/` talks to the M5Stack, GRBL, MFRC522, GoPlus2, HTTP and WiFi libraries.

```
            src/scheduler   src/conveyor   src/sorter   src/api   src/check_connect
                    │             │             │          │            │
                    └─────────────┴──────┬──────┴──────────┴────────────┘
                                         │  interfaces (include/hal/)
                                         ▼
                              src/hal/firmware/   ← ESP32 libraries
```

| Interface        | Firmware implementation        | I2C / network address |
|------------------|--------------------------------|-----------------------|
| `IGrblMotor`     | Module_GRBL 13.2               | `0x70`                |
| `II2CBus`        | Wire (pins 21 / 22)            | —                     |
| `IRfidDriver`    | MFRC522_I2C (reset pin 33)     | `0x28`                |
| `IServoDriver`   | GoPlus2 (servo channel 0)      | `0x38`                |
| `IHttpClient`    | HTTPClient (5 s timeout)       | —                     |
| `INetwork`       | WiFi                           | —                     |

### Sorting bins

| Destination        | Bin  | Servo angle |
|--------------------|------|-------------|
| Warehouse 1        | BIN1 | 0°          |
| Warehouse 2        | BIN2 | 30°         |
| Warehouse 3        | BIN3 | 60°         |
| Error bin          | BIN1 | 0°          |

The error bin is `Bin::ERROR_BIN` in `include/sorter_types.h`. It receives unknown cards and
parcels whose stock movement Dolibarr rejects. It is the same physical bin as warehouse 1,
so the serial log is the only way to tell those parcels apart. The warehouse table is in
`src/sorter/addresser/addresser.cpp`.

## Prerequisites

- [Python 3.12](https://www.python.org/downloads/) (Windows: `winget install Python.Python.3.12`)
- [PlatformIO Core](https://docs.platformio.org/en/latest/core/installation/index.html): `pip install platformio`
- A USB cable and a CP210x driver for the M5Stack
- For the host tests: CMake 3.14+ and a C++17 compiler (GCC, Clang or MSVC)
- Optional: Docker, to run the Dolibarr test instance

## Configuration

Secrets are not committed. Create a `.env` file at the root of the project (the file is ignored by git):

```ini
DOLIBARR_URL=http://your-dolibarr-server/api/index.php
DOLIBARR_API_KEY=your-dolibarr-api-key
WIFI_SSID=your-network
WIFI_PASS=your-password
```

`env_script.py` reads this file before every build and injects the values as compile-time
definitions. The build fails with a clear message if one of them is missing.

The values end up in `firmware.bin`. Anyone who gets the binary can read them, so treat the
binary as sensitive.

## Build the Firmware

```bash
pio run -e m5stack-grey
```

The binary is written to `.pio/build/m5stack-grey/firmware.bin`.

The platform is pinned to `espressif32@6.12.0` in `platformio.ini`. The M5Stack 0.4.6
library needs the Arduino-ESP32 2.x core; newer platform releases ship a 3.x core that
does not build it.

## Releases

Version tags (`v*`) create a GitHub Release with generated notes and the compiled firmware attached as
`firmware-<tag>.bin`, ready to flash. The image contains the Wi-Fi and Dolibarr settings from the
repository secrets. To build it yourself with your own `.env`:

```bash
pio run -e m5stack-grey
```

## Flash and Monitor

Find the serial port, then upload:

```bash
pio device list
pio run -e m5stack-grey -t upload --upload-port COM5
```

Replace `COM5` with your port. To read the logs (115200 baud):

```bash
pio device monitor -e m5stack-grey
```

To clean the build files:

```bash
pio run -e m5stack-grey -t clean
```

## Usage

The M5Stack screen shows the current mode and conveyor state.

| Button | Action                                                       |
|--------|--------------------------------------------------------------|
| **A**  | Start: connection check → position init → production. From `ERROR`, Start acknowledges a latched critical error first. |
| **B**  | Emergency stop: back to `WAITING`, conveyor stopped          |
| **C**  | Show the system and conveyor state                           |

After a critical error the system stops. Check the hardware, then press **A** to restart.

The serial monitor prints every state transition, every hardware change and every error.

## Tests

The unit tests run on the PC, not on the ESP32. They use the fake Arduino core in
`test/host/include/` and gMock doubles for every interface in `include/hal/`.

```bash
cmake -S . -B build -DBUILD_TESTS=ON
cmake --build build
ctest --test-dir build --output-on-failure
```

googletest 1.14.0 and ArduinoJson 7.4.3 are fetched automatically at configure time.

## Dolibarr Setup

The sorting flow needs a Dolibarr instance with the REST API and the products module.
A Docker Compose file is provided for a local instance:

```bash
docker compose up --build
```

Then open `http://localhost:80` and:

1. Enable the module **API / Web services (REST server)** and create an API token (DOLAPIKEY) on your user card.
2. Enable the module **Products**.
3. Create a product extrafield named `default_target_warehouse_id`. The firmware reads it as `options_options_default_target_warehouse_id`; the Swagger spec in `dolibarr_swagger.yaml` documents the shorter key, and both are accepted.

## Continuous Integration

GitHub Actions runs on every push to `main` (`.github/workflows/ci.yml`). Other branches and
pull requests do not trigger it. Concurrent runs for the same branch are cancelled.

- **Unit Tests**: configures and builds the host tests with CMake, then runs them with CTest.
- **Firmware Build (PlatformIO)**: writes `.env` from repository secrets, builds `m5stack-grey` and uploads `firmware.bin` as an artifact.

The firmware job needs these repository secrets: `DOLIBARR_URL`, `DOLIBARR_API_KEY`,
`WIFI_SSID`, `WIFI_PASS`.

## Known Limitations

- **No hardware emergency stop.** Button B stops the motor in software only. A real
  emergency stop has to cut the motor power in hardware.
- **Error bin shared with warehouse 1.** See [Sorting bins](#sorting-bins).
- **Secrets in the binary.** See [Configuration](#configuration).
- **Initialization is a placeholder.** `InitPosition` logs its steps and returns success; it does
  not load motor parameters or home the sorter yet.
- **Not tested on the hardware.** The servo routing, the watchdog and the error bin are covered by
  the host tests only. The hardware checklist in [TEST_PROTOCOL.md](docs/TEST_PROTOCOL.md) has not been run.

## Project Structure

```
.
├── platformio.ini          PlatformIO environment (board, pinned platform, libraries)
├── env_script.py           Injects the .env values into the firmware build
├── CMakeLists.txt          Host build (tests)
├── include/                Module interfaces; include/hal/ holds the hardware interfaces
├── src/
│   ├── main.cpp            setup() / loop(), button handling
│   ├── scheduler/          State machine
│   ├── state_manager/      State observation and rules
│   ├── conveyor/           GRBL motor logic
│   ├── check_connect/      Connection checks
│   ├── init_position/      Position initialization
│   ├── api/                Dolibarr REST client
│   ├── sorter/             Sorter controller, RFID reader, servo, addresser
│   └── hal/firmware/       ESP32 implementations of the hardware interfaces
├── test/host/              Host unit tests, gMock doubles and fake Arduino core
├── docs/                   Architecture, test protocol, usage and build docs
└── lib/GoPlus2/            Vendored GoPlus2 servo driver (third-party, not in the PlatformIO registry)
```

Further documentation is in `docs/`: a system overview (`OVERVIEW.md`), UML diagrams (`uml/`), system usage (`SYSTEM_USAGE.md`), build and test
details (`CMAKE_BUILD_TESTS.md`), the state flow (`state_management_error.md`) and the
test protocol (`TEST_PROTOCOL.md`). `TEST_PROTOCOL.md` is a manual protocol from the school
project; its Python file layout is historical.

## License

Licensed under the [Apache License 2.0](LICENSE).

## Acknowledgements

- [M5Stack](https://github.com/m5stack) libraries (PlatformIO registry) and the GoPlus2 driver (`lib/GoPlus2`)
- [googletest](https://github.com/google/googletest) and [ArduinoJson](https://arduinojson.org)
