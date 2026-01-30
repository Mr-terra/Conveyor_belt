┌─────────────────────────────────────────────────┐
│  1. monitorHardware() (toutes les 50ms)        │
│     ↓                                            │
│  2. readHardwareState()                         │
│     • _grbl.readSpeed()                         │
│     • _grbl.readPosition()                      │
│     • _grbl.getState() (si disponible)          │
│     ↓                                            │
│  3. Mise à jour automatique de conveyorState    │
│     (basé sur hardware)                         │
│     ↓                                            │
│  4. stateManager.monitorAllStates()             │
│     • Détecte le changement d'état              │
│     ↓                                            │
│  5. checkStateRules()                           │
│     • Compare avec les rules définies           │
│     ↓                                            │
│  6. Error::triggerError()                       │
│     • Log l'événement avec le bon niveau        │
└─────────────────────────────────────────────────┘