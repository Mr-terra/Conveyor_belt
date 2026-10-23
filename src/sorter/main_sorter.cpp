// SorterController: runs one sorting cycle (see main_sorter.h).

#include "main_sorter.h"
#include "log.h"

#include <Arduino.h>

#include "sorter_types.h"
#include "reader.h"
#include "servo_sorter.h"
#include "state_manager.h"
#include "error.h"
#include "dolibarr_api.h"


void SorterController::setHardware(IRfidDriver *rfidDriver, IServoDriver *servoDriver, II2CBus *bus)
{
  _rfid.setHardware(rfidDriver, bus);
  _servo.setHardware(servoDriver, bus);
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

void SorterController::setDolibarrAPI(DolibarrAPI *api)
{
  _addresser.setDolibarrAPI(api);
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
  _rfid.init();
  _servo.init(stateManager);

  stateManager.observeState<SorterState>(
      &this->sorterState,
      "SORTER",
      stateToString);

  setupStateRules(stateManager);
  setState(SorterState::Waiting);

  LOG_INFO("Sorter", "ready");
}

bool SorterController::checkConnection()
{
  bool rfidOk = _rfid.checkConnection();
  bool servoOk = _servo.checkConnection();

  if (rfidOk && servoOk)
  {
    LOG_INFO("Sorter", "RFID and servo OK");
    return true;
  }

  LOG_ERROR("Sorter", "RFID or servo check failed");
  return false;
}

// One cycle. No card: WAITING. A card: RFID_READ, find its bin, move the servo, then WAITING.
void SorterController::production()
{
  String uidRead = _rfid.readUid();

  if (uidRead.isEmpty())
  {
    setState(SorterState::Waiting);
    return;
  }

  setState(SorterState::RfidRead);


  // The addresser returns the bin, or Bin::NONE when the card cannot be routed.
  Bin::Id bin = _addresser.resolve(uidRead);
  if (bin == Bin::NONE)
  {
    setState(SorterState::RfidReadingError);
    LOG_WARN("Sorter", "unknown card %s", uidRead.c_str());
    setState(SorterState::Waiting);
  }

  // moveTo() sends Bin::NONE to the error bin, so every parcel ends up in a bin.
  _servo.moveTo(bin);
  setState(SorterState::Waiting);
}
