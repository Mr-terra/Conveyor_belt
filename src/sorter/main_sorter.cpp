#include "main_sorter.h"

#include <Arduino.h>
#include <M5Stack.h>

#include "sorter_types.h"
#include "reader.h"
#include "adresser.h"
#include "servo_sorter.h"
#include "state_manager.h"
#include "error.h"

static RFIDReader rfid;
static Adresser adresser;
static ServoSorter servo;
static String UID;

static String sorterStateToString(String s) { return s; } // signature attendue

void SorterController::setupStateRules(StateManager &stateManager)
{
  stateManager.addStateRule("SORTER", "INIT_PRODUCTION", "WAITING", ERROR_INFO, "WAITING FOR PACKAGE");
  stateManager.addStateRule("SORTER", "WAITING", "RFID_READ", ERROR_INFO, "PACKAGE READ");
  stateManager.addStateRule("SORTER", "WAITING", "RFID_READING_ERROR", ERROR_WARNING, "COULDN'T READ THE PACKAGE");
}

void SorterController::init(StateManager &stateManager)
{
  rfid.init();
  servo.init(stateManager);

  stateManager.observeState<String>(&sorterState, "SORTER", sorterStateToString);

  setupStateRules(stateManager);

  sorterState = "WAITING";

  Serial.println("SorterController ready");
}

bool SorterController::checkConnection()
{
  // Vérifier que RFID et Servo sont connectés
  bool rfidOk = RFIDReader::checkConnection();
  bool servoOk = ServoSorter::checkConnection();
  
  if (rfidOk && servoOk) {
    Serial.println("✅ Sorter system OK (RFID + Servo)");
    return true;
  } else {
    Serial.println("❌ Sorter system failure");
    return false;
  }
}

void SorterController::production()
{
  String uidRead = rfid.readUid();

  if (uidRead.isEmpty())
  {
    // pas de carte -> reste WAITING
    if (sorterState != "WAITING")
      sorterState = "WAITING";
    return;
  }

  // carte lue
  UID = uidRead;
  sorterState = "RFID_READ";

  Serial.print("UID: ");
  Serial.println(uidRead);

  Bin::Id bin = adresser.resolve(uidRead);
  if (bin == Bin::NONE)
  {
    sorterState = "RFID_READING_ERROR";
    Serial.println("Unknown UID");
    // retour à WAITING après l’erreur
    sorterState = "WAITING";
    return;
  }

  servo.moveTo(bin);

  // fin du cycle
  sorterState = "WAITING";
}
