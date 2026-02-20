#pragma once
#include <Arduino.h>
#include "sorter_types.h"
class DolibarrAPI;

class Adresser
{
public:
    Bin::Id resolve(const String &uid) const;
    void setDolibarrAPI(DolibarrAPI *api);

private:
    DolibarrAPI *_api = nullptr;

    String cleanUid(const String &uid) const;
    int extractWarehouseId(const String &json) const;
    int extractProductId(const String &json) const;
};
