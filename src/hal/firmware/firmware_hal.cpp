// Wires the ESP32 adapters into the logic. Called once from setup().
#include "hal/firmware_hal.h"

#include "hal/firmware/adapters.h"

#include "conveyor.h"
#include "dolibarr_api.h"
#include "main_sorter.h"

void bindFirmwareHardware(Conveyor &conveyor, SorterController &sorterController, DolibarrAPI &dolibarrApi)
{
    conveyor.setHardware(grblMotorAdapter(), wireBusAdapter());
    sorterController.setHardware(rfidAdapter(), servoAdapter(), wireBusAdapter());
    dolibarrApi.setTransport(httpClientAdapter(), wifiNetworkAdapter());
}
