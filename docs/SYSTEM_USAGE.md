# User Documentation - RFID Conveyor System

## Table of Contents
1. [System Overview](#system-overview)
2. [Global Architecture](#global-architecture)
3. [Commissioning](#commissioning)
4. [Daily Use](#daily-use)
5. [Error Management](#error-management)
6. [Advanced Configuration](#advanced-configuration)
7. [Troubleshooting](#troubleshooting)

---

## System Overview

The RFID conveyor system is designed to automate parcel sorting using:
- A motorized conveyor (Conveyor)
- An RFID reader to identify the parcels
- An automatic sorting system (Servo Sorter)
- An integration with Dolibarr for stock management
- An automatic state monitoring system

---

## Global Architecture

### Main Components

```
┌───────────────────────────────────────────────────────┐
│                 SCHEDULER (Orchestrator)              │
└───────────────┬───────────────┬───────────────┬───────┘
                │               │               │
                ▼               ▼               ▼
┌─────────────────────┐ ┌─────────────────┐ ┌─────────────────┐
│   CHECK CONNECT    │ │  INIT POSITION  │ │   CONVEYOR      │
└─────────────────────┘ └─────────────────┘ └─────────────────┘
                │                               │
                ▼                               ▼
┌───────────────────────────────────────────────────────┐
│                     SORTER CONTROLLER                  │
│  ┌─────────────┐    ┌─────────────┐    ┌───────────┐   │
│  │ RFID READER │    │  ADRESSER   │    │ SERVO     │   │
│  └─────────────┘    └─────────────┘    └───────────┘   │
└───────────────────────────────────────────────────────┘
                │
                ▼
┌───────────────────────────────────────────────────────┐
│                     DOLIBARR API                      │
└───────────────────────────────────────────────────────┘
```

### Automatic Workflow

```
[WAITING] 
       │ (Button A pressed)
       ▼
[CHECK_CONNECT] → Checks all connections
       │
       ├─ OK → [INIT_POSITIONS] → Initializes the positions
       │                         │
       │                         ├─ OK → [PRODUCTION] → Production mode
       │                         │                         │
       │                         │                         ├─ Conveyor moves forward
       │                         │                         ├─ RFID reads the tags
       │                         │                         └─ Servo sorts the parcels
       │                         │
       └─ KO → [WAITING]        └─ KO → [WAITING]
```

---

## Commissioning

### Prerequisites

1. **Required hardware**:
   - M5Stack with GRBL 13.2 module
   - Compatible RFID reader
   - Servo motor for sorting
   - Stable 12V/24V power supply
   - WiFi network connection

2. **Required software**:
   - PlatformIO or Arduino IDE
   - M5Stack libraries installed
   - Access to the Dolibarr API

### Initial Configuration

#### 1. WiFi and Dolibarr Configuration

Settings are not compiled in. Connect the M5Stack over USB and open the serial monitor at
115200 baud, then send:

```
set wifi <ssid> <password>
set dolibarr <url> <api-key>
show
```

Each `set` saves the setting to flash and restarts the board. `show` prints the settings
without the password or the API key.

#### 2. Hardware Configuration

- **I2C address**: Default 0x70 for the GRBL module
- **Conveyor speed**: Adjustable via `conveyor.setSpeed(1000)` (0-1000)
- **Movement step**: Adjustable via `conveyor.setMovement(1)`

### Startup Procedure

1. **Power**:
   - Connect the 12V/24V power supply
   - Check that all indicator lights are on

2. **Initialization**:
   - The system starts in the `WAITING` state
   - Check the serial monitor: "System initialized. State: WAITING"

3. **Connection check**:
   - Press **Button A** to start the check
   - The system checks:
     - Dolibarr connection
     - Motor connection (I2C)
     - Sorter connection (RFID + Servo)

4. **Position initialization**:
   - If the connections are OK, the system automatically moves to initialization
   - The components move to their starting positions

5. **Production mode**:
   - If the initialization is OK, the system automatically switches to production
   - The conveyor starts moving
   - The system is ready to read RFID tags

---

## Daily Use

### User Interface (M5Stack Buttons)

| Button | Function | Description |
|--------|----------|-------------|
| **A**  | Start    | Starts the automatic process (check → initialization → production) |
| **B**  | Stop     | Stops the system and returns to the WAITING state |
| **C**  | Info     | Shows the current state of the system and the conveyor |

### Normal Production Cycle

1. **Preparation**:
   - Make sure the conveyor is empty
   - Check that all indicator lights are green
   - Press **Button C** to check the state: "WAITING"

2. **Start**:
   - Press **Button A** to start
   - The system automatically performs:
     - Connection check (≈5-10s)
     - Position initialization (≈3-5s)
     - Switch to production mode

3. **Production**:
   - Place the parcels on the conveyor
   - The system automatically reads the RFID tags
   - The parcels are sorted according to the configuration
   - The conveyor keeps moving

4. **Stop**:
   - Press **Button B** to stop
   - The conveyor stops gradually
   - The system returns to the WAITING state

### Visual Indicators

| State | Indicator | Description |
|------|------------|-------------|
| WAITING | Blue LED | System ready, waiting to start |
| CHECK_CONNECT | Flashing yellow LED | Connection check in progress |
| INIT_POSITIONS | Solid yellow LED | Position initialization in progress |
| PRODUCTION | Green LED | Production mode active |
| ERROR | Flashing red LED | System error (see the serial monitor) |

---

## Error Management

### Common Errors and Solutions

| Error | Probable Cause | Solution |
|--------|----------------|----------|
| "Conveyor not initialized" | I2C connection problem | Check the GRBL module wiring |
| "RFID not detected" | RFID reader not connected | Check the RFID wiring |
| "Dolibarr connection FAILED" | Network or Dolibarr API down | Check the WiFi and the API URL |
| "Servo not responding" | Servo not powered | Check the servo power supply |
| "GRBL Motor not detected" | Wrong I2C address | Check the address (0x70 by default) |

### Restart Procedure

1. **Emergency stop**:
   - Press **Button B** to stop the system
   - If the system does not respond, cut the power

2. **Diagnosis**:
   - Connect the serial monitor (115200 baud)
   - Read the error messages
   - Identify the faulty component

3. **Restart**:
   - Fix the identified problem
   - Turn the power off and back on
   - The system restarts in the WAITING state

---

## Advanced Configuration

### Adjusting the Conveyor Speed

```cpp
// In setup() after conveyor.initialize():
conveyor.setSpeed(800);  // Value between 0 (stop) and 1000 (max)
```

### Adjusting the Movement Step

```cpp
// In setup() after conveyor.initialize():
conveyor.setMovement(2);  // Number of steps per movement
```

### Configuring the Sorting Bins

Edit in `src/sorter/adresser/adresser.cpp`:

```cpp
// Mapping between RFID UIDs and destination bins
Bin Adresser::resolve(String uid) {
    if (uid == "04:1A:2B:3C:4D:5E:6F") return Bin::BIN_1;
    if (uid == "04:1A:2B:3C:4D:5F:7A") return Bin::BIN_2;
    // Add your mappings here
    return Bin::NONE; // Unknown
}
```

### Enabling/Disabling Dolibarr

```cpp
// To disable Dolibarr (offline mode):
// In setup(), do not configure the Dolibarr API:
// checkConnect.setDolibarrAPI(nullptr);
```

---

## Troubleshooting

### I2C Connection Problems

1. Check the wiring: SDA → GPIO21, SCL → GPIO22
2. Check the 5V power supply of the GRBL module
3. Test with an I2C scanner:
   ```cpp
   // Add in setup() for diagnosis:
   Wire.begin();
   byte error, address;
   int nDevices = 0;
   for(address = 1; address < 127; address++ ) {
       Wire.beginTransmission(address);
       error = Wire.endTransmission();
       if (error == 0) {
           Serial.print("I2C device found at 0x");
           Serial.println(address, HEX);
           nDevices++;
       }
   }
   if (nDevices == 0) Serial.println("No I2C devices found");
   ```

### RFID Problems

1. Check that the RFID antenna is properly connected
2. Test with a known tag
3. Check the distance (max 5-10cm depending on the tag)
4. Try another RFID tag

### Network Problems

1. Check the WiFi connection
2. Test the Dolibarr URL in a browser
3. Check that the API key is valid
4. Test with a tool such as Postman

### Serial Monitor

**Configuration**: 115200 baud

**Useful commands**:
- `BtnC`: Shows the current state
- Error messages are detailed
- State transitions are logged

---

## Preventive Maintenance

### Daily Checks
1. Clean the conveyor rollers
2. Check the belt tension
3. Test the RFID reading with a known tag
4. Check the servo movement

### Weekly Checks
1. Lubricate the bearings
2. Check the electrical connections
3. Test the emergency stop button
4. Update the firmware if needed

### Maintenance Log

Keep a log with:
- Date and time
- Problem encountered
- Solution applied
- Parts replaced
- Downtime

---

## Glossary

- **GRBL**: Open-source firmware for motor control
- **I2C**: Communication bus for peripherals
- **RFID**: Radio-frequency identification
- **UID**: Unique identifier of an RFID tag
- **API**: Programming interface for Dolibarr
- **State Manager**: Automatic state management system

---

## Support

For any question or problem:
- Check the technical documentation in `docs/`
- Check the examples in `examples/`
- Contact the support team: support@conveyor-system.com

---

**Version**: 1.0
**Last updated**: 2023-11-15
**Author**: Conveyor RFID Team
