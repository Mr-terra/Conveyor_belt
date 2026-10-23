#pragma once

// Accessors for the ESP32 adapters. Each adapter lives in its own file in src/hal/firmware/.
// Every accessor returns the single instance of its adapter, created on first use.
// The logic never includes this header: only bindFirmwareHardware() uses it.

#include "hal/grbl_motor.h"
#include "hal/http_client.h"
#include "hal/i2c_bus.h"
#include "hal/network.h"
#include "hal/rfid_driver.h"
#include "hal/servo_driver.h"

IGrblMotor *grblMotorAdapter();
II2CBus *wireBusAdapter();
IRfidDriver *rfidAdapter();
IServoDriver *servoAdapter();
IHttpClient *httpClientAdapter();
INetwork *wifiNetworkAdapter();
