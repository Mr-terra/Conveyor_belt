# Host Build, Tests and CI

The firmware is built with PlatformIO. CMake builds only the host unit tests: the firmware logic
compiled for the PC, with the hardware replaced by mocks. This document explains how the build is
organised, how to run the tests, and what the CI does.

## 1. Layout

```
.
├── CMakeLists.txt            Top level: C++17, enable_testing(), adds test/ when BUILD_TESTS=ON
├── platformio.ini            Firmware build (not part of CMake)
├── include/
│   ├── hal/                  Hardware interfaces (I2C bus, GRBL, RFID, servo, HTTP, network)
│   └── *.h                   Module headers
├── src/
│   ├── hal/firmware/         ESP32 implementations of include/hal/ (firmware only)
│   └── ...                   Module sources (compiled for the host and the firmware)
└── test/
    ├── CMakeLists.txt        Fetches googletest and ArduinoJson, builds firmware_logic and host_tests
    └── host/
        ├── include/Arduino.h Fake Arduino core: String, Serial, millis/delay, constrain
        ├── fakes/hardware_mocks.h   gMock doubles for every interface in include/hal/
        └── tests/*_test.cpp  One gtest file per module
```

`src/hal/firmware/` is deliberately left out of the host build. It only compiles for the ESP32.

## 2. Build options

| Option | Default | Effect |
|--------|---------|--------|
| `BUILD_TESTS` | `ON` | Adds `test/` (googletest, the host logic library and `host_tests`) |

There is no firmware option in CMake: the firmware is built with `pio run -e m5stack-grey`.

## 3. Dependencies

Fetched at configure time by `FetchContent`; nothing needs to be installed system-wide:

| Dependency | Version | Used for |
|------------|---------|----------|
| googletest (with gMock) | 1.14.0 | Test framework and mocks |
| ArduinoJson | 7.4.3 | JSON parsing in the logic (same major version as the firmware) |

You still need CMake 3.14 or newer and a C++17 compiler.

## 4. Building and running the tests

From the project root:

```bash
cmake -S . -B build -DBUILD_TESTS=ON
cmake --build build
ctest --test-dir build --output-on-failure
```

To run a single test:

```bash
ctest --test-dir build -R SchedulerTest --output-on-failure
```

On Windows with the Visual Studio generator, add `--config Release` to the build and test commands.

### Compiler flags

The `firmware_logic` library is compiled with `-Wall -Wextra -Werror`, so warnings in the logic
fail the build the same way they would on the firmware.

## 5. How the host build compiles the logic

`test/CMakeLists.txt` lists the logic sources explicitly in `HOST_LOGIC_SOURCES`:

- `firmware_logic` is a static library built from those sources.
- `host_tests` is the test executable. It links `firmware_logic` and `gmock_main`.
- `gtest_discover_tests(host_tests)` registers every test with CTest, so each test name shows up
  as its own line in the CTest output.

The DOLIBARR endpoint and key are set as compile definitions for the host build:

```cmake
"DOLIBARR_URL=\"http://localhost/api/index.php\""
"DOLIBARR_API_KEY=\"test-api-key\""
```

A new module that the logic depends on must be added to `HOST_LOGIC_SOURCES`, or its tests cannot
link. A new hardware interface needs a mock in `test/host/fakes/hardware_mocks.h`.

## 6. Writing a test

Each test file includes the real module header and the mocks, and drives the module through the
interfaces only:

```cpp
class ExampleTest : public ::testing::Test {
protected:
    NiceMock<MockGrblMotor> motor;   // defaults: every call returns a neutral value
    NiceMock<MockI2CBus> bus;
    Conveyor conveyor{10, 500};
    void SetUp() override { conveyor.setHardware(&motor, &bus); }
};

TEST_F(ExampleTest, AlarmStatusMapsToAlarm) {
    EXPECT_CALL(motor, readStatus()).WillOnce(Return(String("A")));
    EXPECT_EQ(conveyor.readHardwareState(), CONVEYOR_STATE_ALARM);
}
```

Tips taken from this project:

- Use `NiceMock` for devices the test does not care about, so unexpected calls are ignored.
- Do not combine `ON_CALL` and `EXPECT_CALL` with the same matcher on one mock without care: a
  `WillOnce` chain is easier to read than two overlapping expectations.
- Time is controlled with the fake clock: set `hostMillis` before calling a function that reads
  `millis()`.
- Critical errors are a global latch. Clear them in `SetUp()` and `TearDown()` so tests do not
  affect each other.

## 7. Continuous integration

The workflow is `.github/workflows/ci.yml`. It runs on pushes to `main` only, and a newer push
cancels a run still in progress.

| Job | Steps |
|-----|-------|
| **Unit Tests** | Checkout, configure with `-DBUILD_TESTS=ON`, build, `ctest --test-dir build --output-on-failure` |
| **Firmware Build (PlatformIO)** | Checkout, Python 3.11, cache `~/.platformio` (keyed on `platformio.ini`), install PlatformIO, write `.env` from repository secrets, `pio run -e m5stack-grey`, upload `firmware.bin` as an artifact |

The unit-test job uses the cmake and compiler already on the `ubuntu-latest` runner.

The firmware job needs the repository secrets `DOLIBARR_URL`, `DOLIBARR_API_KEY`, `WIFI_SSID` and
`WIFI_PASS`. Without them, `env_script.py` stops the build with a message that names the missing
variable.

The firmware is pinned to `espressif32@6.12.0` (Arduino-ESP32 2.x). Newer platform releases
ship a 3.x core that does not build the M5Stack library; see `platformio.ini`.

## 8. Useful commands

```bash
# Reconfigure from scratch
rm -rf build && cmake -S . -B build -DBUILD_TESTS=ON

# List the tests without running them
ctest --test-dir build -N

# Firmware
pio run -e m5stack-grey            # build
pio run -e m5stack-grey -t clean   # clean
```
