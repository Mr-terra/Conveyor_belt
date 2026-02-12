#include "adresser.h"

// TODO : ADAPTER LA PROCEDURE POUR TRIER LES COLIS
//  + si data like 00:00:AA:?? alors boite 1 ?
Bin::Id Adresser::resolve(const String &uid) const
{
    //call to dolibard to get warehouse
    if (uid == "AA:BB:CC:DD")
        //call to dolibard to move goods to warehouse X
        return Bin::BIN1;
    if (uid == "11:22:33:44")
        // call to dolibard to move goods to warehouse X
        return Bin::BIN2;
    if (uid == "DE:AD:BE:EF")
        // call to dolibard to move goods to warehouse X
        return Bin::BIN3;
    return Bin::NONE;
}