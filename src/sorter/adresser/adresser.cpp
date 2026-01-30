#include "adresser.h"

// TODO : ADAPTER LA PROCEDURE POUR TRIER LES COLIS
//  + si data like 00:00:AA:?? alors boite 1 ?
Bin::Id Adresser::resolve(const String &uid) const
{
    if (uid == "AA:BB:CC:DD")
        return Bin::BIN1;
    if (uid == "11:22:33:44")
        return Bin::BIN2;
    if (uid == "DE:AD:BE:EF")
        return Bin::BIN3;
    return Bin::NONE;
}