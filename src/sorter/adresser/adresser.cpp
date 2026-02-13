#include "adresser.h"

// TODO : ADAPTER LA PROCEDURE POUR TRIER LES COLIS
//  + si data like 00:00:AA:?? alors boite 1 ?
Bin::Id Adresser::resolve(const String &uid) const
{
    Serial.print("Wazup bro, i am adressing your package \n");
    // call to dolibard to get warehouse
    if (uid == "04:1C:19:DA:70:00:00")
        // call to dolibard to move goods to warehouse X
        return Bin::BIN1;
    if (uid == "04:1D:19:DA:70:00:00")
        // call to dolibard to move goods to warehouse X
        return Bin::BIN2;
    if (uid == "04:1B:19:DA:70:00:00")
        // call to dolibard to move goods to warehouse X
        return Bin::BIN3;
    return Bin::NONE;
}