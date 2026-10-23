#include "main_sorter.h"

#include <Arduino.h>

#include "sorter_types.h"
#include "reader.h"
#include "servo_sorter.h"
#include "state_manager.h"
#include "error.h"
#include "globals.h"

static RFIDReader rfid;
static ServoSorter servo;

SorterController sorterController;

void SorterController::setHardware(IRfidDriver *rfidDriver, IServoDriver *servoDriver, II2CBus *bus)
{
  rfid.setHardware(rfidDriver, bus);
  servo.setHardware(servoDriver, bus);
}

static String sorterStateToString(String s)
{
  return s.isEmpty() ? String("") : s;
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
      sorterStateToString);

  setupStateRules(stateManager);
  setState("WAITING");

  Serial.println("SorterController ready");
}

bool SorterController::checkConnection()
{
  bool rfidOk = rfid.checkConnection();
  bool servoOk = servo.checkConnection();

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

  Bin::Id bin = addresser.resolve(uidRead);
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
