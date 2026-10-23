#pragma once

#include "dolibarr_api.h"
#include "addresser.h"

#ifndef DOLIBARR_URL
#error "DOLIBARR_URL not defined"
#endif
#ifndef DOLIBARR_API_KEY
#error "DOLIBARR_API_KEY not defined"
#endif

extern DolibarrAPI dolibarrApi;
extern Addresser addresser;