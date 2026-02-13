# Run the project

source venv/bin/activate

make build    # compiler le projet
make upload   # uploader sur le M5Stack
make clean    # nettoyer les fichiers compilés

# State Management Example

This example demonstrates the complete architecture for automatic state monitoring and error handling with proper separation of concerns.

## Architecture Overview

```
┌───────────────────────────────────────────────────────────────┐
│                    STATE ERROR MANAGER                         │
│  (Pure Observer - No Business Logic Dependencies)              │
└───────────────────────────────────────────────────────────────┘
                              ▲
                              │ Observes
                              ▼
┌─────────────────┐    ┌─────────────────┐    ┌─────────────────┐
│     CONVEYOR     │    │   EXAMPLE MODULE │    │    SCHEDULER    │
│  (Business Logic)│    │  (Business Logic)│    │  (Business Logic)│
└─────────────────┘    └─────────────────┘    └─────────────────┘
       ▲                     ▲                     ▲
       │ Self-Config         │ Self-Config         │ Self-Config
       ▼                     ▼                     ▼
┌───────────────────────────────────────────────────────────────┐
│                    STATE VARIABLES                             │
│  (Global variables observed by StateManager)              │
└───────────────────────────────────────────────────────────────┘
```

## Key Components

### 1. StateManager (Pure Observer)
- **Responsibility**: Monitor state variables and apply rules
- **Does NOT**: Call business objects directly
- **Uses**: Callbacks to notify about errors

### 2. Business Modules (Conveyor, ExampleModule, etc.)
- **Responsibility**: Manage their own state and business logic
- **Provide**: State variables and conversion functions
- **Configure**: Their own state monitoring rules

### 3. Callbacks
- **ErrorCallback**: Handles non-critical errors
- **CriticalErrorCallback**: Handles critical errors and safety measures

## How It Works

### Module Self-Configuration
Each module configures its own state monitoring:

```cpp
void Conveyor::initializeWithStateMonitoring(StateManager& StateManager) {
    initialize(); // Basic initialization
    configureStateMonitoring(StateManager, &conveyorState); // Auto-config
}
```

### State Monitoring Configuration
Each module defines its own rules:

```cpp
void Conveyor::setupStateRules(StateManager& StateManager) {
    // Define expected transitions
    StateManager.addStateRule("Conveyor", "STOPPED", "RUNNING", ERROR_INFO, 
        "Conveyor started successfully");
    
    // Define forbidden transitions
    StateManager.addForbiddenTransition("Conveyor", "ERROR", "ERROR", 
        "Conveyor stuck in error state");
}
```

### Automatic Monitoring
StateManager automatically detects all state changes:

```cpp
void loop() {
    // Modules check their own state integrity
    conveyor.checkState();
    
    // Update state variables
    conveyorState = conveyor.getState();
    
    // StateManager detects changes and applies rules
    StateManager.monitorAllStates();
}
```

## Example Output

```
=== STATE MANAGEMENT EXAMPLE ===
Demonstrating clean architecture with automatic state monitoring

=== SYSTEM READY ===
All modules configured with automatic state monitoring
State changes will be automatically detected and validated

--- Demonstration Cycle ---

1. Testing Conveyor operations:
State change detected: Conveyor from STOPPED to RUNNING
📋 Error: [Conveyor] INFO: Conveyor started successfully

2. Testing ExampleModule operations:
State change detected: ExampleModule from IDLE to WORKING
📋 Error: [ExampleModule] INFO: ExampleModule started working

3. Testing error conditions:
Simulating conveyor motor failure...
State change detected: Conveyor from RUNNING to ERROR
📋 Error: [Conveyor] CRITICAL: Conveyor entered error state from running
🚨 CRITICAL ERROR: Safety measures executed
```

## Creating New Modules

To add a new module to this architecture:

1. **Define States**: Create state constants and conversion function
2. **Self-Configuration**: Implement `configureStateMonitoring()`
3. **State Rules**: Define expected transitions in `setupStateRules()`
4. **State Integrity**: Implement `checkState()` for business logic
5. **Initialization**: Call `initializeWithStateMonitoring()`

## Benefits

✅ **Automatic State Monitoring**: No manual state checking needed
✅ **Clean Architecture**: Proper separation of concerns
✅ **Easy Extension**: Simple to add new modules
✅ **Automatic Error Detection**: Rules and transitions validated automatically
✅ **No Circular Dependencies**: Modules don't know about StateManager
✅ **Flexible Error Handling**: Change callbacks without modifying observation

## Key Principles

1. **Single Responsibility**: Each component has one clear job
2. **Separation of Concerns**: Observation vs. Action are separate
3. **No Circular Dependencies**: Clean dependency graph
4. **Module Autonomy**: Each module configures itself
5. **Automatic Validation**: State transitions are validated automatically

This architecture provides a robust foundation for building reliable systems with automatic state monitoring and error handling.