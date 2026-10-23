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

String SorterController::stateToString(SorterState s)
{
  switch (s)
  {
  case SorterState::InitProduction:
    return "INIT_PRODUCTION";
  case SorterState::Waiting:
    return "WAITING";
  case SorterState::RfidRead:
    return "RFID_READ";
  case SorterState::RfidReadingError:
    return "RFID_READING_ERROR";
  }
  return "UNKNOWN";
}

void SorterController::setState(SorterState newState)
{
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

  stateManager.observeState<SorterState>(
      &this->sorterState,
      "SORTER",
      stateToString);

  setupStateRules(stateManager);
  setState(SorterState::Waiting);

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
    setState(SorterState::Waiting);
    return;
  }

  setState(SorterState::RfidRead);


  Bin::Id bin = addresser.resolve(uidRead);
  if (bin == Bin::NONE)
  {
    setState(SorterState::RfidReadingError);
    Serial.println("Unknown UID");
    setState(SorterState::Waiting);
  }

  servo.moveTo(bin);
  setState(SorterState::Waiting);
}
