#pragma once
#include "../types.h"

class ServoSorter
{
public:
    void init();
    void moveTo(Bin::Id bin);

private:
    int angleFor(Bin::Id bin) const;
};
