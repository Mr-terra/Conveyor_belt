#include "hal/device_config.h"
#include <Preferences.h>

static const char *PREFS_NAMESPACE = "conveyor";

static void storeString(const char *key, const String &value)
{
    Preferences prefs;
    prefs.begin(PREFS_NAMESPACE, false);
    prefs.putString(key, value);
    prefs.end();
}

DeviceConfig loadDeviceConfig()
{
    DeviceConfig config;
    Preferences prefs;
    prefs.begin(PREFS_NAMESPACE, true);
    config.wifiSsid = prefs.getString("wifi_ssid", "");
    config.wifiPass = prefs.getString("wifi_pass", "");
    config.dolibarrUrl = prefs.getString("dolibarr_url", "");
    config.dolibarrApiKey = prefs.getString("dolibarr_key", "");
    prefs.end();
    return config;
}

void printProvisioningHelp()
{
    Serial.println("Settings are missing. Send one line per setting over serial (115200 baud):");
    Serial.println("  set wifi <ssid> <password>");
    Serial.println("  set dolibarr <url> <api-key>");
    Serial.println("  show");
}

// Splits "<first> <rest>" at the first space. The rest may contain spaces.
static bool splitFirstWord(const String &text, String &first, String &rest)
{
    int space = text.indexOf(' ');
    if (space < 0)
    {
        first = text;
        rest = "";
    }
    else
    {
        first = text.substring(0, space);
        rest = text.substring(space + 1);
        rest.trim();
    }
    return !first.isEmpty();
}

static void handleSet(const String &args)
{
    String target, value;
    if (!splitFirstWord(args, target, value))
    {
        printProvisioningHelp();
        return;
    }

    if (target == "wifi")
    {
        String ssid, password;
        if (!splitFirstWord(value, ssid, password))
        {
            printProvisioningHelp();
            return;
        }
        storeString("wifi_ssid", ssid);
        storeString("wifi_pass", password);
        Serial.println("Wi-Fi settings saved, restarting");
    }
    else if (target == "dolibarr")
    {
        String url, key;
        if (!splitFirstWord(value, url, key) || key.isEmpty())
        {
            printProvisioningHelp();
            return;
        }
        storeString("dolibarr_url", url);
        storeString("dolibarr_key", key);
        Serial.println("Dolibarr settings saved, restarting");
    }
    else
    {
        printProvisioningHelp();
        return;
    }

    delay(100); // let the serial output flush before the restart
    ESP.restart();
}

static void showSettings()
{
    DeviceConfig config = loadDeviceConfig();
    // Passwords and API keys are never printed
    Serial.println("Wi-Fi SSID: " + (config.hasWifi() ? config.wifiSsid : String("(not set)")));
    Serial.println("Dolibarr URL: " + (config.dolibarrUrl.isEmpty() ? String("(not set)") : config.dolibarrUrl));
    Serial.println("Dolibarr API key: " + String(config.dolibarrApiKey.isEmpty() ? "(not set)" : "(set)"));
}

void processProvisioningInput()
{
    if (!Serial.available())
        return;

    String line = Serial.readStringUntil('\n');
    line.trim();

    if (line.startsWith("set "))
        handleSet(line.substring(4));
    else if (line == "show")
        showSettings();
    else if (!line.isEmpty())
        printProvisioningHelp();
}
