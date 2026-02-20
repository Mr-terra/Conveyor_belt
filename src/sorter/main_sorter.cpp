#include "main_sorter.h"

#include <Arduino.h>
#include <M5Stack.h>

#include "sorter_types.h"
#include "reader.h"
#include "servo_sorter.h"
#include "state_manager.h"
#include "error.h"
#include "globals.h"

static RFIDReader rfid;
static ServoSorter servo;

SorterController sorterController;

// ✅ VRAIE signature utilisée par le StateManager.cpp (String*)
static String sorterStateToStringPtr(String *s)
{
  return (s && s->length()) ? *s : String("");
}

void SorterController::setState(const char *newState)
{
  if (sorterState != newState)
    sorterState = newState;
}

void SorterController::setupStateRules(StateManager &stateManager)
{
  stateManager.addStateRule("SORTER", "INIT_PRODUCTION", "WAITING", ERROR_INFO, "WAITING FOR PACKAGE");
  stateManager.addStateRule("SORTER", "WAITING", "RFID_READ", ERROR_INFO, "PACKAGE READ");
  stateManager.addStateRule("SORTER", "WAITING", "RFID_READING_ERROR", ERROR_WARNING, "COULDN'T READ THE PACKAGE");
}

void SorterController::initialize(StateManager &stateManager)
{
  rfid.init();
  servo.init(stateManager);

  stateManager.observeState<String>(
      &this->sorterState,
      "SORTER",
      reinterpret_cast<String (*)(String)>(sorterStateToStringPtr));

  setupStateRules(stateManager);
  setState("WAITING");

  Serial.println("SorterController ready");
}

bool SorterController::checkConnection()
{
  bool rfidOk = RFIDReader::checkConnection();
  bool servoOk = ServoSorter::checkConnection();

  if (rfidOk && servoOk)
  {
    Serial.println("✅ Sorter system OK (RFID + Servo)");
    return true;
  }

  Serial.println("❌ Sorter system failure");
  return false;
}

void SorterController::production()
{
  String uidRead = rfid.readUid();

  if (uidRead.isEmpty())
  {
    setState("WAITING");
    return;
  }

  setState("RFID_READ");

  Serial.print("UID: ");
  Serial.println(uidRead);

  Bin::Id bin = adresser.resolve(uidRead);
  Serial.print("Bin return: ");
  Serial.println((int)bin);
  if (bin == Bin::NONE)
  {
    setState("RFID_READING_ERROR");
    Serial.println("Unknown UID");
    setState("WAITING");
  }

  servo.moveTo(bin);
  setState("WAITING");
}
