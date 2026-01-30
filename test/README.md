# State and Error Management System Test

This test file demonstrates the automatic state monitoring and error handling system.

## Features Tested

1. **Automatic State Monitoring**: The system automatically detects state changes and applies transition rules
2. **Error Level Handling**: Different error levels (INFO, WARNING, CRITICAL, FATAL) trigger appropriate responses
3. **Automatic Recovery**: The system attempts to recover from certain error conditions automatically
4. **State Transition Rules**: Comprehensive rules for all state transitions

## Test Scenarios

1. **Normal Operation**: Tests smooth state transitions
2. **Error Conditions**: Tests how the system handles errors
3. **Recovery Procedures**: Tests automatic recovery mechanisms
4. **Manual Error Triggering**: Tests direct error triggering

## How to Use

1. Upload this test file to your Arduino/PlatformIO environment
2. Open the Serial Monitor (9600 baud)
3. Observe the automatic state monitoring and error handling
4. The test repeats every 10 seconds

## Expected Output

- State change detection messages
- Error messages with appropriate levels
- Automatic recovery attempts
- Safety measure execution for critical errors

## Integration with Main System

The main system uses the same StateManager but with more comprehensive state transition rules defined in `setupStateMonitoring()` in `main.cpp`.