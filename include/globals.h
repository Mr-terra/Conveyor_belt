#pragma once

#include "dolibarr_api.h"
#include "adresser.h"

#ifndef DOLIBARR_URL
#error "DOLIBARR_URL not defined"
#endif
#ifndef DOLIBARR_API_KEY
#error "DOLIBARR_API_KEY not defined"
#endif

extern DolibarrAPI dolibarrApi;
extern Adresser adresser;