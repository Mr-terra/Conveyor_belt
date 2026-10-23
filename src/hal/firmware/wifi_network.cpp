// Adapter: the ESP32 Wi-Fi link behind the INetwork interface.
#include "hal/firmware/adapters.h"

#include <WiFi.h>

namespace
{
    class WifiNetwork : public INetwork
    {
    public:
        bool isConnected() override { return WiFi.status() == WL_CONNECTED; }
        String localIp() override { return WiFi.localIP().toString(); }
        String gatewayIp() override { return WiFi.gatewayIP().toString(); }
    };
}

INetwork *wifiNetworkAdapter()
{
    static WifiNetwork network;
    return &network;
}
