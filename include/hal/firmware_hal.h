#pragma once

// Wires the real ESP32 hardware drivers into the firmware logic.
// Must be called once from setup(), before any module is initialized.
void bindFirmwareHardware();
