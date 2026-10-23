#include <Arduino.h>
#include <WiFi.h>
#include "conveyor.h"
#include "scheduler.h"
#include "check_connect.h"
#include "init_position.h"
#include "state_manager.h"
#include "main_sorter.h"
#include "error.h"
#include "dolibarr_api.h"
#include "addresser.h"
#include "globals.h"

#ifndef WIFI_SSID
#error "WIFI_SSID not defined"
#endif
#ifndef WIFI_PASS
#error "WIFI_PASS not defined"
#endif

extern Conveyor conveyor;
extern Scheduler scheduler;
extern CheckConnect checkConnect;
extern InitPosition initPosition;
extern StateManager stateManager;
extern SorterController sorterController;

static bool dolibarrChecked = false;

static bool waitWifi(uint32_t timeoutMs = 15000)
{
    uint32_t start = millis();
    while (WiFi.status() != WL_CONNECTED)
    {
        if (millis() - start > timeoutMs)
            return false;
        delay(200);
    }
    return true;
}

void setup()
{
    Serial.begin(115200);
    Wire.begin(21, 22);
    Wire.setClock(400000);
    M5.begin();

    conveyor.initialize(stateManager);
    sorterController.initialize(stateManager);
    scheduler.initialize();
    initPosition.initialize();
    addresser.setDolibarrAPI(&dolibarrApi);
    Serial.printf("[SETUP] &addresser=%p  &dolibarrApi=%p\n", (void *)&addresser, (void *)&dolibarrApi);

    // 1) WiFi
    WiFi.mode(WIFI_STA);
    WiFi.begin(WIFI_SSID, WIFI_PASS);

    if (!waitWifi())
    {
        Serial.println("❌ WiFi connect timeout");
    }
    else
    {
        Serial.print("✅ WiFi OK, IP=");
        Serial.println(WiFi.localIP());
    }

    // 2) Inject ALL dependencies BEFORE initializing the scheduler
    checkConnect.setDolibarrAPI(&dolibarrApi);
    checkConnect.setConveyor(&conveyor);
    checkConnect.setSorterController(&sorterController);
    checkConnect.checkDolibarrConnection();

    scheduler.setDependencies(&checkConnect, &initPosition, &sorterController, &conveyor);

    stateManager.setScheduler(&scheduler);


    // UI
    M5.Lcd.clear();
    M5.Lcd.setTextSize(2);
    M5.Lcd.setCursor(10, 10);
    M5.Lcd.println("Conveyor Control");
    M5.Lcd.setCursor(10, 40);
    M5.Lcd.println("A: Forward  C: Backward");
    M5.Lcd.setCursor(10, 70);
    M5.Lcd.println("B: Toggle Mode");
    M5.Lcd.setCursor(10, 100);
    M5.Lcd.print("Mode: ");
    M5.Lcd.println(scheduler.getState());

    scheduler.triggerStop();

    Serial.println("System initialized. State: WAITING");
}

void updateDisplay()
{
    M5.Lcd.setCursor(10, 100);
    M5.Lcd.print("Mode: ");
    M5.Lcd.println(scheduler.getState());

    M5.Lcd.setCursor(10, 130);
    M5.Lcd.print("State: ");
    M5.Lcd.println(conveyor.getState());
}

void loop()
{
    M5.update();

    conveyor.monitorHardware();
    stateManager.monitorAllStates();

    // Check Dolibarr ONCE, when WiFi is really connected
    if (!dolibarrChecked && WiFi.status() == WL_CONNECTED)
    {
        dolibarrChecked = true;
        bool ok = dolibarrApi.checkConnection();
        Serial.println(ok ? "✅ Dolibarr OK" : "❌ Dolibarr FAIL");
    }

    // Button handling
    if (M5.BtnA.wasPressed())
    {
        Serial.println("🔄 Button A pressed - Start Production Sequence");
        if (scheduler.getState() == STATE_WAITING)
        {
            // Start the full sequence: check → initialization → production
            scheduler.triggerCheckConnect();
        }
        else if (scheduler.getState() == STATE_PRODUCTION)
        {
            Serial.println("⚠️  Production already running");
        }
        else
        {
            Serial.println("⚠️  System busy with " + scheduler.getState() + " - wait or use emergency stop");
        }
        updateDisplay();
    }
    else if (M5.BtnB.wasPressed())
    {
        Serial.println("🔄 Button B pressed - EMERGENCY STOP");
        scheduler.triggerStop();
        updateDisplay();
    }
    else if (M5.BtnC.wasPressed())
    {
        Serial.println("🔄 Button C pressed - System Info/Status");
        // Show useful system information
        M5.Lcd.setCursor(10, 190);
        M5.Lcd.print("Info: ");
        M5.Lcd.print(scheduler.stateToString(scheduler.getState()));
        M5.Lcd.print(" | Conv: ");
        M5.Lcd.print(conveyor.getState());
        delay(2000); // Show the info for 2 seconds
        updateDisplay(); // Back to the normal display
    }

    scheduler.executeCurrentState();
    delay(100);
}