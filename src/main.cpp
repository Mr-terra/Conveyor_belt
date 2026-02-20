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
#include "adresser.h"
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
    adresser.setDolibarrAPI(&dolibarrApi);
    Serial.printf("[SETUP] &adresser=%p  &dolibarrApi=%p\n", (void *)&adresser, (void *)&dolibarrApi);

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

    // 2) Injecte TOUTES les deps AVANT d'initialiser le scheduler
    checkConnect.setDolibarrAPI(&dolibarrApi);
    checkConnect.setConveyor(&conveyor);
    checkConnect.setSorterController(&sorterController);
    checkConnect.checkDolibarrConnection();

    scheduler.setDependencies(&checkConnect, &initPosition, &sorterController, &conveyor);

    // 3) Maintenant seulement tu lances le scheduler
    scheduler.initialize();

    initPosition.initialize();

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

    // Check Dolibarr UNE FOIS quand WiFi est vraiment OK
    if (!dolibarrChecked && WiFi.status() == WL_CONNECTED)
    {
        dolibarrChecked = true;
        bool ok = dolibarrApi.checkConnection();
        Serial.println(ok ? "✅ Dolibarr OK" : "❌ Dolibarr FAIL");
    }

    conveyor.monitorHardware();
    stateManager.monitorAllStates();

    if (M5.BtnA.wasPressed())
    {
        Serial.println("🔄 Bouton A pressed - Forward");
        conveyor.move(true);
    }
    else if (M5.BtnC.wasPressed())
    {
        Serial.println("🔄 Bouton C pressed - Backward");
        conveyor.move(false);
    }
    else if (M5.BtnB.wasPressed())
    {
        Serial.println("🔄 Bouton B pressed - Toggle Mode");
        if (scheduler.getState() == "WAITING")
            scheduler.triggerProduction();
        else
            scheduler.triggerStop();
    }

    scheduler.executeCurrentState();
    delay(100);
}