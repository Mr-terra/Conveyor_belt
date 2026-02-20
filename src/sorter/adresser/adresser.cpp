#include "adresser.h"
#include "dolibarr_api.h"
#include <ArduinoJson.h>

static constexpr int WAREHOUSE_1 = 1;
static constexpr int WAREHOUSE_2 = 2;
static constexpr int WAREHOUSE_3 = 3;

void Adresser::setDolibarrAPI(DolibarrAPI *api)
{
    _api = api;
}

Bin::Id Adresser::resolve(const String &uid) const
{
    Serial.printf("[RESOLVE] this=%p  _api=%p\n", (void *)this, (void *)_api);
    if (_api == nullptr)
    {
        Serial.println("[ERR] DolibarrAPI not set (Adresser::_api is null)");
        return Bin::NONE;
    }

    String trimmedUid = cleanUid(uid);
    String jsonResponse = _api->getProductByBarcode(trimmedUid);

    if (jsonResponse.length() == 0)
    {
        Serial.println("[ERR] Empty HTTP response");
        return Bin::NONE;
    }
    Serial.print(jsonResponse);

    int warehouseId = extractWarehouseId(jsonResponse);
    int productId = extractProductId(jsonResponse);

    Serial.print("Warehouse ID: ");
    Serial.println(warehouseId);
    Serial.print("Product ID: ");
    Serial.println(productId);

    if (warehouseId == WAREHOUSE_1)
    {
        _api->createStockMovement(productId, warehouseId, 1);
        return Bin::BIN1;
    }
    if (warehouseId == WAREHOUSE_2)
    {
        _api->createStockMovement(productId, warehouseId, 1);
        return Bin::BIN2;
    }
    if (warehouseId == WAREHOUSE_3)
    {
        _api->createStockMovement(productId, warehouseId, 1);
        return Bin::BIN3;
    }

    return Bin::NONE;
}

String Adresser::cleanUid(const String &uid) const
{
    String result = uid;
    result.replace(":", "");

    return "'" + result + "'";
}

int Adresser::extractWarehouseId(const String &json) const
{
    DynamicJsonDocument doc(4096);

    DeserializationError error = deserializeJson(doc, json);

    if (error)
    {
        Serial.print("JSON parse failed: ");
        Serial.println(error.c_str());
        return -1;
    }

    // Ton JSON est un tableau → on prend l’index 0
    if (!doc.is<JsonArray>() || doc.size() == 0)
    {
        Serial.println("[ERR] JSON is not array or empty");
        return -1;
    }

    JsonObject product = doc[0].as<JsonObject>();

    const char *warehouseId =
        product["array_options"]["options_options_default_target_warehouse_id"];

    if (warehouseId == nullptr)
        return -1;

    return atoi(warehouseId);
}

int Adresser::extractProductId(const String &json) const
{
    DynamicJsonDocument doc(4096);

    DeserializationError error = deserializeJson(doc, json);
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