#pragma once
#include <Arduino.h>
#include "sorter_types.h"
class DolibarrAPI;

class Addresser
{
public:
    Addresser() : _api(nullptr) {}
    void setDolibarrAPI(DolibarrAPI *api);
    Bin::Id resolve(const String &uid) const;

private:
    DolibarrAPI *_api;

    String cleanUid(const String &uid) const;
    int extractWarehouseId(const String &json) const;
    int extractProductId(const String &json) const;
};
