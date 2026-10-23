#include "addresser.h"
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
        Serial.println("[ERR] DolibarrAPI not set (Addresser::_api is null)");
        return Bin::NONE;
    }

    String trimmedUid = cleanUid(uid);
    String jsonResponse = _api->getProductByBarcode(trimmedUid);

    if (jsonResponse.length() == 0)
    {
        Serial.println("[ERR] Empty HTTP response");
        return Bin::NONE;
    }

    int warehouseId = extractWarehouseId(jsonResponse);
    int productId = extractProductId(jsonResponse);


    for (const WarehouseBin &route : WAREHOUSE_BINS)
    {
        if (warehouseId != route.warehouseId)
            continue;

        // The parcel is diverted even when Dolibarr rejects the movement: the stock
        // count is then wrong, and the error log is the only trace of it.
        if (!_api->createStockMovement(productId, warehouseId, 1))
        {
            Serial.println("[ERR] Stock movement was not recorded in Dolibarr - sending to the error bin");
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
        Serial.print("JSON parse failed: ");
        Serial.println(error.c_str());
        return -1;
    }

    // Your JSON is an array → we take index 0
    if (!doc.is<JsonArray>() || doc.size() == 0)
    {
        Serial.println("[ERR] JSON is not array or empty");
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
        Serial.print("JSON parse failed: ");
        Serial.println(error.c_str());
        return -1;
    }

    if (!doc.is<JsonArray>() || doc.size() == 0)
    {
        Serial.println("[ERR] JSON is not array or empty");
        return -1;
    }

    JsonObject product = doc[0].as<JsonObject>();

    const char *productId = product["id"];

    if (productId == nullptr)
        return -1;

    return atoi(productId);
}
