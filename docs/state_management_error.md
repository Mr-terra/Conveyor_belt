# 🔄 State Flow Architecture

## 📊 System Overview

The system uses a **3-layer** architecture to manage the states:

```
┌─────────────────────────────────────────────────────────────┐
│                         MAIN.CPP                             │
│                    (Main Orchestrator)                       │
│                                                              │
│  • Calls monitorHardware() every 10ms                        │
│  • Calls monitorAllStates() every 10ms                       │
│  • Handles the scheduler's business logic                    │
│  • Sends user commands (start/stop)                          │
└─────────────────────────────────────────────────────────────┘
        ↓ Commands              ↑ State reading
        ↓                        ↑
┌─────────────────────────────────────────────────────────────┐
│                      CONVEYOR.CPP                            │
│                  (Physical Hardware Module)                  │
│                                                              │
│  State variable: _currentState (private String)              │
│  • "STOPPED" - Motor stopped (GRBL = 'I')                    │
│  • "RUNNING" - Motor moving (GRBL = 'R')                     │
│  • "ALARM"   - Module locked (GRBL = 'A')                    │
│                                                              │
│  Responsibilities:                                           │
│  1. Read the hardware via _grbl.readStatus()                 │
│  2. Map the GRBL codes → internal states                     │
│  3. Write into _currentState                                 │
│  4. Execute the commands (start/stop/move)                   │
└─────────────────────────────────────────────────────────────┘
        ↑ Observed via pointer
        ↑
┌─────────────────────────────────────────────────────────────┐
│                    STATE_MANAGER.CPP                         │
│                  (Automatic Monitor)                         │
│                                                              │
│  State monitor: StateMonitor                                 │
│  • void* stateVariable → Points to _currentState             │
│  • String previousState → Remembers the previous state       │
│                                                              │
│  Responsibilities:                                           │
│  1. Read *stateVariable (dereference the pointer)            │
│  2. Compare currentState vs previousState                    │
│  3. If different → Apply the rules (StateRule)               │
│  4. Trigger the errors via Error::triggerError()             │
└─────────────────────────────────────────────────────────────┘
```

---

## 🎬 Full Sequence of a State Change

### Phase 1: Initialization (setup)

```cpp
void setup() {
    M5.begin();
    
    // 1️⃣ Initialize the Conveyor module
    conveyor.initialize(stateManager);
}
```

**In `conveyor.initialize()`:**

```cpp
void Conveyor::initialize(StateManager& stateManager) {
    // 1. Initialize the GRBL hardware
    _grbl.Init(&Wire);
    _grbl.setMode("distance");
    
    // 2. Read the initial state from the hardware
    readHardwareState();
    //   ↓
    //   └─> _currentState = "STOPPED" (for example)
    
    // 3. Register the automatic monitoring
    configureStateMonitoring(stateManager);
    //   ↓
    //   ├─> stateManager.observeState(&_currentState, "Conveyor", ...)
    //   │    ↓
    //   │    └─> The StateManager stores:
    //   │         • Pointer to _currentState
    //   │         • previousState = "STOPPED"
    //   │         • stateToString() callback
    //   │
    //   └─> setupStateRules(stateManager)
    //        ↓
    //        └─> Registers the transition rules:
    //             STOPPED → RUNNING = ERROR_INFO
    //             * → ALARM = ERROR_FATAL
    //             etc.
}
```

**Result after setup:**

```
StateManager.stateMonitors[0] = {
    stateVariable: 0x3FFC2A40,  // Address of conveyor._currentState
    moduleName: "Conveyor",
    previousState: "STOPPED",
    lastChangeTime: 1234,
    isStuckWarningGiven: false,
    stateToString: Conveyor::stateToString
}
```

---

### Phase 2: Main Loop (loop)

```cpp
void loop() {
    M5.update();
    
    // ✨ STEP 1: Read the hardware
    conveyor.monitorHardware();
    
    // ✨ STEP 2: Detect the changes
    stateManager.monitorAllStates();
    
    // User commands
    if (M5.BtnA.wasPressed()) {
        conveyor.start();
    }
    
    // Continuous movement
    if (conveyor.getState() == CONVEYOR_STATE_RUNNING) {
        conveyor.move(true);
    }
    
    delay(10);
}
```

---

### STEP 1: `conveyor.monitorHardware()`

**Responsibility: Synchronize the internal state with the real hardware**

```cpp
void Conveyor::monitorHardware() {
    static unsigned long lastCheck = 0;
    
    if (millis() - lastCheck >= 100) {  // Throttled to 100ms
        readHardwareState();
        //   ↓
        //   └─> READS the state of the GRBL module
        //        ↓
        //        └─> UPDATES _currentState if a change is detected
        
        lastCheck = millis();
    }
}
```

**Detail of `readHardwareState()`:**

```cpp
String Conveyor::readHardwareState() {
    // 1. Query the GRBL module
    String status = _grbl.readStatus();
    //                     ↑
    //              Returns "I" or "R" or "A"
    
    // 2. Map the GRBL code to a state
    String newState;
    if (status[0] == 'I') {
        newState = CONVEYOR_STATE_STOPPED;  // "STOPPED"
    } else if (status[0] == 'A') {
        newState = CONVEYOR_STATE_ALARM;    // "ALARM"
    } else {
        newState = CONVEYOR_STATE_RUNNING;  // "RUNNING"
    }
    
    // 3. Update IF CHANGED
    if (newState != _currentState) {
        Serial.print("🔄 Hardware: ");
        Serial.print(_currentState);  // e.g. "STOPPED"
        Serial.print(" → ");
        Serial.println(newState);     // e.g. "RUNNING"
        
        _currentState = newState;  // ← WRITE into the observed variable
        //    ↑
        //    The StateManager will detect this change
        //    because it holds a pointer to this variable
    }
    
    return newState;
}
```

**System state after STEP 1:**

```
Memory 0x3FFC2A40 (conveyor._currentState) = "RUNNING"
                                               ↑
                        The content has changed, but the StateManager
                        has not detected it yet
```

---

### STEP 2: `stateManager.monitorAllStates()`

**Responsibility: Detect the changes and apply the rules**

```cpp
void StateManager::monitorAllStates() {
    for (StateMonitor& monitor : stateMonitors) {
        
        // 1️⃣ READ the current state through the pointer
        String* statePtr = static_cast<String*>(monitor.stateVariable);
        //      ↑
        //      = 0x3FFC2A40 (address of conveyor._currentState)
        
        String currentState = monitor.stateToString(*statePtr);
        //                                           ↑
        //                    Dereferences the pointer → reads "RUNNING"
        //                                           ↓
        //                    stateToString("RUNNING") → returns "RUNNING"
        
        // 2️⃣ COMPARE with the previous state
        if (currentState != monitor.previousState) {
            //  "RUNNING"   !=  "STOPPED"  → TRUE !
            
            // 🎉 CHANGE DETECTED!
            Serial.print("🔄 State change detected: ");
            Serial.print(monitor.moduleName);       // "Conveyor"
            Serial.print(" from ");
            Serial.print(monitor.previousState);    // "STOPPED"
            Serial.print(" to ");
            Serial.println(currentState);           // "RUNNING"
            
            // 3️⃣ APPLY the rules
            checkStateRules("Conveyor", "STOPPED", "RUNNING");
            //   ↓
            //   └─> Looks for a matching rule:
            //        module="Conveyor" + fromState="STOPPED" + toState="RUNNING"
            //        ↓
            //        Rule found:
            //        {
            //            module: "Conveyor",
            //            fromState: "STOPPED",
            //            toState: "RUNNING",
            //            level: ERROR_INFO,
            //            message: "Motor started (detected from hardware)"
            //        }
            //        ↓
            //        Error::triggerError("Motor started...", ERROR_INFO, "Conveyor");
            //        ↓
            //        Serial: "ℹ️ [Conveyor] Motor started (detected from hardware)"
            
            // 4️⃣ REMEMBER the new state
            monitor.previousState = currentState;  // "RUNNING"
            monitor.lastChangeTime = millis();
            monitor.isStuckWarningGiven = false;
        }
        else {
            // 5️⃣ NO CHANGE: check whether it is stuck
            if (millis() - monitor.lastChangeTime > 10000) {  // 10s
                if (currentState != "WAITING" && currentState != "STOPPED") {
                    String msg = monitor.moduleName + " stuck in: " + currentState;
                    Error::triggerError(msg, ERROR_WARNING, monitor.moduleName);
                    monitor.isStuckWarningGiven = true;
                }
            }
        }
    }
}
```

**Result after STEP 2:**

```
StateManager.stateMonitors[0].previousState = "RUNNING"  // Updated
Console: "ℹ️ [Conveyor] Motor started (detected from hardware)"
```

---

## 🔁 Full Cycle: Starting the Motor

### Detailed Timeline

```
T=0ms
------
main.loop()
  ↓
  M5.update()
  ↓
  conveyor.monitorHardware()
    ↓
    readHardwareState()
      ↓
      _grbl.readStatus() → "I"  (Idle)
      ↓
      _currentState = "STOPPED"  (no change)
  ↓
  stateManager.monitorAllStates()
    ↓
    currentState = "STOPPED"
    previousState = "STOPPED"
    ↓
    No change detected
  ↓
  delay(10)


T=500ms
-------
main.loop()
  ↓
  M5.update()
  ↓
  M5.BtnA.wasPressed() → TRUE  // The user presses BtnA
  ↓
  conveyor.start()
    ↓
    _isRunning = true
    ↓
    // No change to _currentState here!
    // The hardware decides the state


T=600ms
-------
main.loop()
  ↓
  conveyor.monitorHardware()
    ↓
    readHardwareState()
      ↓
      _grbl.readStatus() → "R"  (Running) ← The hardware has changed!
      ↓
      newState = "RUNNING"
      ↓
      newState != _currentState  → TRUE
      ↓
      Serial: "🔄 Hardware: STOPPED → RUNNING"
      ↓
      _currentState = "RUNNING"  ← WRITE!
                      ↑
                      At address 0x3FFC2A40


T=610ms
-------
main.loop()
  ↓
  stateManager.monitorAllStates()
    ↓
    *statePtr = "RUNNING"  ← READ at address 0x3FFC2A40
    ↓
    currentState = "RUNNING"
    previousState = "STOPPED"
    ↓
    currentState != previousState  → TRUE
    ↓
    Serial: "🔄 State change detected: Conveyor from STOPPED to RUNNING"
    ↓
    checkStateRules("Conveyor", "STOPPED", "RUNNING")
      ↓
      Rule found: STOPPED → RUNNING = ERROR_INFO
      ↓
      Error::triggerError("Motor started...", ERROR_INFO, "Conveyor")
      ↓
      Serial: "ℹ️ [Conveyor] Motor started (detected from hardware)"
    ↓
    monitor.previousState = "RUNNING"  ← REMEMBER


T=620ms
-------
main.loop()
  ↓
  if (conveyor.getState() == CONVEYOR_STATE_RUNNING) {
      //     ↑ Returns _currentState = "RUNNING"
      conveyor.move(true);
      ↓
      _grbl.setMotor(1, 0, 0, 1000);  ← Movement command
  }
```

---

## 📋 Responsibilities Comparison Table

| Component | Role | Writes _currentState? | Reads _currentState? | Triggers Errors? |
|-----------|------|----------------------|--------------------|--------------------|
| **main.cpp** | General orchestration | ❌ | ✅ (via getState()) | ❌ |
| **conveyor.cpp** | Hardware interface | ✅ (readHardwareState) | ✅ (getState) | ❌ |
| **state_manager.cpp** | Automatic monitoring | ❌ | ✅ (via pointer) | ✅ (via rules) |

---

## 🎯 Key Points of the Architecture

### 1. **Single Source of Truth: The Hardware**

```cpp
// ❌ BAD APPROACH (old)
void Conveyor::start() {
    _grbl.setMotor(...);
    _currentState = "RUNNING";  // ← Assumes it worked
}

// ✅ GOOD APPROACH (current)
void Conveyor::start() {
    _grbl.setMotor(...);
    // No change to _currentState here!
    // readHardwareState() will handle it on the next read
}
```

**Advantage**: If the hardware refuses to start (alarm, error), the state automatically stays at "STOPPED".

---

### 2. **Read/Write Separation**

```
conveyor.cpp          → WRITES into _currentState (based on the hardware)
state_manager.cpp     → READS _currentState (via pointer)
main.cpp              → READS _currentState (via getState())
```

**Advantage**: No risk of write conflicts.

---

### 3. **Detection by Comparison**

```cpp
// StateManager keeps a COPY of the previous state
monitor.previousState = "STOPPED";

// On each cycle, compare with the current state
if (currentState != monitor.previousState) {
    // Change detected!
}
```

**Advantage**: Triggers the rules only **once** per transition.

---

### 4. **Declarative Rules**

```cpp
// Define the rules in setup
stateManager.addStateRule("Conveyor", "STOPPED", "RUNNING", 
                          ERROR_INFO, "Motor started");

// They are applied automatically in monitorAllStates()
```

**Advantage**: All the monitoring logic is centralized.

---

## 🔍 Simplified Data Flow

```
┌─────────┐
│  GRBL   │  Physical hardware
│ Module  │  Real state: 'I', 'R', 'A'
└────┬────┘
     │ readStatus()
     ↓
┌─────────────────────────┐
│ Conveyor::readHardwareState() │
│                         │
│ Mapping:                │
│  'I' → "STOPPED"        │
│  'R' → "RUNNING"        │
│  'A' → "ALARM"          │
│                         │
│ ↓ Writes into           │
│ _currentState           │
└────┬────────────────────┘
     │ Address: 0x3FFC2A40
     ↓
┌─────────────────────────┐
│ StateManager            │
│                         │
│ Reads via pointer:      │
│ *statePtr = _currentState│
│                         │
│ Compares:               │
│ currentState ≠ previousState│
│                         │
│ ↓ If different          │
│ checkStateRules()       │
└────┬────────────────────┘
     ↓
┌─────────────────────────┐
│ Error::triggerError()   │
│                         │
│ Displays the message    │
│ Applies the actions     │
└─────────────────────────┘
```

---

## 🛠️ Debug Example

### Adding Traces to Check the Flow

```cpp
// In conveyor.cpp
String Conveyor::readHardwareState() {
    String newState = /* ... */;
    
    if (newState != _currentState) {
        Serial.print("[CONVEYOR] Write to address ");
        Serial.print((unsigned long)&_currentState, HEX);
        Serial.print(": ");
        Serial.print(_currentState);
        Serial.print(" → ");
        Serial.println(newState);
        
        _currentState = newState;
    }
    return newState;
}
```

```cpp
// In state_manager.cpp
void StateManager::monitorAllStates() {
    for (StateMonitor& monitor : stateMonitors) {
        String* statePtr = static_cast<String*>(monitor.stateVariable);
        
        Serial.print("[STATE_MANAGER] Read from address ");
        Serial.print((unsigned long)statePtr, HEX);
        Serial.print(": ");
        Serial.println(*statePtr);
        
        String currentState = monitor.stateToString(*statePtr);
        // ...
    }
}
```

**Expected output:**

```
[CONVEYOR] Write to address 3FFC2A40: STOPPED → RUNNING
[STATE_MANAGER] Read from address 3FFC2A40: RUNNING
🔄 State change detected: Conveyor from STOPPED to RUNNING
ℹ️ [Conveyor] Motor started (detected from hardware)
```

**Proof that both point to the same address!** ✅

---

## 📚 Flow Summary

### Flow 1: User Command → Hardware

```
BtnA pressed
  ↓
main.cpp: conveyor.start()
  ↓
conveyor.cpp: _grbl.setMotor(...)
  ↓
GRBL module receives the command
  ↓
Hardware physically starts
```

### Flow 2: Hardware → Internal State

```
GRBL module moving
  ↓
conveyor.cpp: monitorHardware()
  ↓
readHardwareState(): _grbl.readStatus() → "R"
  ↓
Mapping: "R" → "RUNNING"
  ↓
_currentState = "RUNNING"
```

### Flow 3: Internal State → Detection → Action

```
_currentState = "RUNNING" (at address 0x3FFC2A40)
  ↓
state_manager.cpp: monitorAllStates()
  ↓
Reads via pointer: *statePtr → "RUNNING"
  ↓
Compares: "RUNNING" ≠ "STOPPED" (previousState)
  ↓
checkStateRules("Conveyor", "STOPPED", "RUNNING")
  ↓
Rule found: ERROR_INFO
  ↓
Error::triggerError("Motor started...", ERROR_INFO, "Conveyor")
  ↓
Console: "ℹ️ [Conveyor] Motor started (detected from hardware)"
```

---

## 🎓 Conclusion

**3-layer architecture:**

1. **main.cpp**: Orchestrator that coordinates everything
2. **conveyor.cpp**: Hardware expert that reads and writes the real state
3. **state_manager.cpp**: Watcher that observes and reacts

**Core principle:**
> The hardware is the source of truth.  
> The code adapts to the hardware, not the other way around.

**Benefits:**
- ✅ Automatic detection of hardware problems
- ✅ No possible desynchronization
- ✅ Declarative and maintainable code
- ✅ Centralized monitoring logic

---
