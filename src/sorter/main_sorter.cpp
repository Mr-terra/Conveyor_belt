#include "main_sorter.h"

#include <Arduino.h>
#include <M5Stack.h>

#include "types.h"
#include "rfid_reader/reader.h"
#include "adresser/adresser.h"
#include "servo_sorter/servo_sorter.h"

static RFIDReader  rfid;
static Adresser    adresser;
static ServoSorter servo;

void SorterController::init() {
  rfid.init();
  servo.init();
  Serial.println("SorterController ready");
}

void SorterController::update() {
  String uid = rfid.readUid();
  if (uid.isEmpty()) return;

  Serial.print("UID: ");
  Serial.println(uid);

  Bin::Id bin = adresser.resolve(uid);
  if (bin == Bin::NONE) {
    Serial.println("Unknown UID");
    return;
  }

  Serial.print("Target: ");
  Serial.println(Bin::toString(bin));

  servo.moveTo(bin);
}
