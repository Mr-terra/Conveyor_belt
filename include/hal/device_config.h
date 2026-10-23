#pragma once

#include <Arduino.h>

// Connection settings kept in the ESP32 flash (NVS). They are entered once over
// the serial console, so no secret is compiled into the firmware binary.
struct DeviceConfig
{
    String wifiSsid;
    String wifiPass;
    String dolibarrUrl;
    String dolibarrApiKey;

    bool hasWifi() const { return !wifiSsid.isEmpty(); }
    bool hasDolibarr() const { return !dolibarrUrl.isEmpty() && !dolibarrApiKey.isEmpty(); }
};

DeviceConfig loadDeviceConfig();
void printProvisioningHelp();

// Reads serial lines and applies the provisioning commands. Call from loop().
// A saved setting takes effect after the restart that follows it.
void processProvisioningInput();
