#pragma once
#include <Arduino.h>

// Hardware abstraction for the WiFi link.
class INetwork
{
public:
    virtual ~INetwork() = default;

    virtual bool isConnected() = 0;
    virtual String localIp() = 0;
    virtual String gatewayIp() = 0;
};
