#pragma once
#include <Arduino.h>
#include "sorter_types.h"

class Adresser
{
public:
    Bin::Id resolve(const String &uid) const;
};
