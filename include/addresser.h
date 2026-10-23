// Addresser: maps a scanned card to the sorter bin of its destination warehouse.

#pragma once
#include <Arduino.h>
#include "sorter_types.h"
class DolibarrAPI;

// Maps a scanned card to a sorter bin.
// Steps: clean the UID, look the product up in Dolibarr, read its target warehouse,
// record a stock movement, and return the bin for that warehouse (see resolve()).
class Addresser
{
public:
    Addresser() : _api(nullptr) {}
    // Must be called before resolve(). Without it, resolve() returns Bin::NONE.
    void setDolibarrAPI(DolibarrAPI *api);
    // Returns the bin for the card with this UID.
    // Bin::NONE means the card could not be routed; the caller then sends it to the error bin.
    Bin::Id resolve(const String &uid) const;

private:
    DolibarrAPI *_api;

    // Removes the ':' separators and wraps the UID in quotes, as the Dolibarr SQL filter expects.
    String cleanUid(const String &uid) const;
    // Reads the warehouse id from the product JSON. Returns -1 if it is missing or the JSON is invalid.
    int extractWarehouseId(const String &json) const;
    // Reads the product id from the product JSON. Returns -1 if it is missing or the JSON is invalid.
    int extractProductId(const String &json) const;
};
