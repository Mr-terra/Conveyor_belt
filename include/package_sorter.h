#ifndef PACKAGE_SORTER_H
#define PACKAGE_SORTER_H

#include <Arduino.h>

class PackageSorter {
public:
    void initialize();
    String defineDirection(String packageInfo);
    void sortPackage(String packageInfo);
};

#endif