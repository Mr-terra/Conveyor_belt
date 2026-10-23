┌─────────────────────────────────────────────────┐
│  1. monitorHardware() (every 50ms)              │
│     ↓                                           │
│  2. readHardwareState()                         │
│     • _grbl.readSpeed()                         │
│     • _grbl.readPosition()                      │
│     • _grbl.getState() (if available)           │
│     ↓                                           │
│  3. Automatic update of conveyorState           │
│     (based on hardware)                         │
│     ↓                                           │
│  4. stateManager.monitorAllStates()             │
│     • Detects the state change                  │
│     ↓                                           │
│  5. checkStateRules()                           │
│     • Compares with the defined rules           │
│     ↓                                           │
│  6. Error::triggerError()                       │
│     • Logs the event with the right level       │
└─────────────────────────────────────────────────┘
