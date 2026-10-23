# UML diagrams

Diagrams of the firmware, written in PlantUML. Each `.puml` file has a rendered `.png` next to it
(GitHub shows the PNG; the PlantUML source is the one to edit).

| File | Kind | Shows |
|------|------|-------|
| `01_architecture` | Component | Logic in `src/`, HAL interfaces in `include/hal/`, firmware adapters in `src/hal/firmware/` |
| `02_state_rfid_cooldown` | State | RFID reader cooldown (1.5 s after an accepted read) |
| `03_state_scheduler` | State | `WAITING`, `CHECK_CONNECT`, `INIT_POSITIONS`, `PRODUCTION`, `ERROR` and their transitions |
| `04_state_conveyor` | State | `STOPPED`, `RUNNING`, `ALARM` and the silent-module counter |
| `05_state_servo` | State | Servo `INIT`, `IDLE`, `MOVING` and the bin angles |
| `06_class_diagram` | Class | HAL interfaces, firmware adapters and the logic classes with their main members |
| `07_sequence_startup` | Sequence | `setup()`: hardware binding, module initialization, Wi-Fi, Dolibarr check |
| `08_sequence_sort_parcel` | Sequence | One pass of `PRODUCTION`: card read, Dolibarr lookup, stock movement, servo |
| `09_sequence_alarm` | Sequence | Conveyor alarm, the critical latch, the operator restart with Button A |
| `10_activity_loop` | Activity | One pass of `loop()` |
| `11_deployment` | Deployment | M5Stack, I2C devices, conveyor motor, servo, Wi-Fi, Dolibarr server |
| `12_use_case` | Use case | Operator, maintainer and the Dolibarr server |

## Regenerate the PNG files

PlantUML needs Java. Download `plantuml.jar` once from the PlantUML releases, then from the
project root:

```bash
java -jar plantuml.jar -tpng docs/uml/*.puml
```

Check the syntax without writing images:

```bash
java -jar plantuml.jar -checkonly docs/uml/*.puml
```
