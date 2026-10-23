// Entry point of the firmware.
// setup() starts the hardware, Wi-Fi and the modules once.
// loop() runs every pass: it reads the hardware, applies the state rules, handles the buttons,
// and runs the work of the current state.

#include <Arduino.h>
#include "log.h"
#include <M5Stack.h>
#include <WiFi.h>
#include <Wire.h>
#include "conveyor.h"
#include "scheduler.h"
#include "check_connect.h"
#include "init_position.h"
#include "state_manager.h"
#include "main_sorter.h"
#include "error.h"
#include "dolibarr_api.h"
#include "hal/firmware_hal.h"

#ifndef WIFI_SSID
#error "WIFI_SSID not defined"
#endif
#ifndef WIFI_PASS
#error "WIFI_PASS not defined"
#endif

// The application objects. They are created here, in the one file that wires them together,
// and passed to the modules by pointer. Arduino keeps no state between loop() calls, so they
// must live for the whole program.
static DolibarrAPI dolibarrApi(DOLIBARR_URL, DOLIBARR_API_KEY);
static StateManager stateManager;
static Conveyor conveyor(10, 250);
static SorterController sorterController;
static Scheduler scheduler;
static CheckConnect checkConnect;
static InitPosition initPosition;

// The Dolibarr check runs once, the first time Wi-Fi is connected.
static bool dolibarrChecked = false;

// Waits up to timeoutMs for Wi-Fi. Returns false on timeout.
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

// Start-up order matters: serial and I2C, then the hardware binding, then the module initialization,
// then Wi-Fi, and last the scheduler's dependencies.
void setup()
{
    Serial.begin(115200);
    Wire.begin(21, 22);
    Wire.setClock(400000);
    M5.begin();

    bindFirmwareHardware(conveyor, sorterController, dolibarrApi);

    conveyor.initialize(stateManager);
    sorterController.initialize(stateManager);
    scheduler.initialize();
    initPosition.initialize();
    sorterController.setDolibarrAPI(&dolibarrApi);

    // 1) WiFi
    WiFi.mode(WIFI_STA);
    WiFi.begin(WIFI_SSID, WIFI_PASS);

    if (!waitWifi())
    {
        LOG_ERROR("Main", "WiFi connect timeout");
    }
    else
    {
        LOG_INFO("Main", "WiFi connected, IP %s", WiFi.localIP().toString().c_str());
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
    M5.Lcd.println(scheduler.stateToString(scheduler.getState()));

    scheduler.triggerStop();

    LOG_INFO("Main", "system initialized, state WAITING");
}

// Redraws the mode and the conveyor state on the screen.
void updateDisplay()
{
    M5.Lcd.setCursor(10, 100);
    M5.Lcd.print("Mode: ");
    M5.Lcd.println(scheduler.stateToString(scheduler.getState()));

    M5.Lcd.setCursor(10, 130);
    M5.Lcd.print("State: ");
    M5.Lcd.println(conveyor.stateToString(conveyor.getState()));
}

// One pass, about every 100 ms.
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
        if (ok)
            LOG_INFO("Main", "Dolibarr OK");
        else
            LOG_ERROR("Main", "Dolibarr check failed");
    }

    // Button handling
    if (M5.BtnA.wasPressed())
    {
        LOG_INFO("Main", "button A pressed: start");
        if (scheduler.getState() == SchedulerState::Waiting || scheduler.getState() == SchedulerState::Error)
        {
            // Start is the operator's acknowledgement: a latched critical error is cleared here,
            // never by the automatic recovery. The checks run next and stop the system again if needed.
            Error::clearCriticalErrors();
            // Start the full sequence: check → initialization → production
            scheduler.triggerCheckConnect();
        }
        else if (scheduler.getState() == SchedulerState::Production)
        {
            LOG_WARN("Main", "production already running");
        }
        else
        {
            LOG_WARN("Main", "system busy in state %s, wait or press B", scheduler.stateToString(scheduler.getState()).c_str());
        }
        updateDisplay();
    }
    else if (M5.BtnB.wasPressed())
    {
        LOG_WARN("Main", "button B pressed: stop");
        scheduler.triggerStop();
        updateDisplay();
    }
    else if (M5.BtnC.wasPressed())
    {
        LOG_INFO("Main", "button C pressed: status");
        // Show useful system information
        M5.Lcd.setCursor(10, 190);
        M5.Lcd.print("Info: ");
        M5.Lcd.print(scheduler.stateToString(scheduler.getState()));
        M5.Lcd.print(" | Conv: ");
        M5.Lcd.print(conveyor.stateToString(conveyor.getState()));
        delay(2000); // Show the info for 2 seconds
        updateDisplay(); // Back to the normal display
    }

    // Runs the work of the current state (checks, initialization, or one production step).
    scheduler.executeCurrentState();
    delay(100);
}
