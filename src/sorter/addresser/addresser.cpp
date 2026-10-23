// Addresser: turns a card UID into a Dolibarr product and a sorter bin (see addresser.h).

#include "addresser.h"
#include "log.h"
#include "dolibarr_api.h"
#include <ArduinoJson.h>

// Dolibarr warehouse id -> sorter bin. Add a row to route another warehouse.
struct WarehouseBin
{
    int warehouseId;
    Bin::Id bin;
};

static constexpr WarehouseBin WAREHOUSE_BINS[] = {
    {1, Bin::BIN1},
    {2, Bin::BIN2},
    {3, Bin::BIN3},
};

void Addresser::setDolibarrAPI(DolibarrAPI *api)
{
    _api = api;
}

Bin::Id Addresser::resolve(const String &uid) const
{
    if (_api == nullptr)
    {
        LOG_ERROR("Addresser", "Dolibarr API not set");
        return Bin::NONE;
    }

    String trimmedUid = cleanUid(uid);
    String jsonResponse = _api->getProductByBarcode(trimmedUid);

    if (jsonResponse.length() == 0)
    {
        LOG_ERROR("Addresser", "empty Dolibarr response");
        return Bin::NONE;
    }

    // The JSON is parsed once per field. This is simple to read, and fine for one card at a time.
    int warehouseId = extractWarehouseId(jsonResponse);
    int productId = extractProductId(jsonResponse);


    // Finds the bin of the warehouse. A warehouse that is not in the table gives Bin::NONE.
    for (const WarehouseBin &route : WAREHOUSE_BINS)
    {
        if (warehouseId != route.warehouseId)
            continue;

        // The parcel is diverted even when Dolibarr rejects the movement: the stock
        // count is then wrong, and the error log is the only trace of it.
        if (!_api->createStockMovement(productId, warehouseId, 1))
        {
            LOG_ERROR("Addresser", "stock movement rejected, parcel sent to the error bin");
            return Bin::ERROR_BIN;
        }
        return route.bin;
    }

    return Bin::NONE;
}

String Addresser::cleanUid(const String &uid) const
{
    String result = uid;
    result.replace(":", "");

    return "'" + result + "'";
}

int Addresser::extractWarehouseId(const String &json) const
{
    JsonDocument doc;

    DeserializationError error = deserializeJson(doc, json.c_str());

    if (error)
    {
        LOG_ERROR("Addresser", "JSON parse failed: %s", error.c_str());
        return -1;
    }

    // Dolibarr returns an array of matching products. The first one is used.
    if (!doc.is<JsonArray>() || doc.size() == 0)
    {
        LOG_ERROR("Addresser", "product response is not a non-empty array");
        return -1;
    }

    JsonObject product = doc[0].as<JsonObject>();

    const char *warehouseId =
        product["array_options"]["options_options_default_target_warehouse_id"];

    // The Swagger spec documents the key without the double "options_" prefix
    if (warehouseId == nullptr)
        warehouseId = product["array_options"]["options_default_target_warehouse_id"];

    if (warehouseId == nullptr)
        return -1;

    return atoi(warehouseId);
}

int Addresser::extractProductId(const String &json) const
{
    JsonDocument doc;

    DeserializationError error = deserializeJson(doc, json.c_str());
    if (error)
    {
        LOG_ERROR("Addresser", "JSON parse failed: %s", error.c_str());
        return -1;
    }

    if (!doc.is<JsonArray>() || doc.size() == 0)
    {
        LOG_ERROR("Addresser", "product response is not a non-empty array");
        return -1;
    }

    JsonObject product = doc[0].as<JsonObject>();

    const char *productId = product["id"];

    if (productId == nullptr)
        return -1;

    return atoi(productId);
}
