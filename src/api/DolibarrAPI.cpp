#include "dolibarr_api.h"

DolibarrAPI::DolibarrAPI(String url, String key)
{
    baseUrl = url;
    apiKey = key;
}

String DolibarrAPI::performGET(String url)
{
    HTTPClient http;
    http.begin(url);
    http.addHeader("DOLAPIKEY", apiKey);

    int httpCode = http.GET();
    String response = "";

    if (httpCode > 0)
    {
        response = http.getString();
    }

    http.end();
    return response;
}

String DolibarrAPI::performPOST(String url, String body)
{
    HTTPClient http;
    http.begin(url);

    http.addHeader("Content-Type", "application/json");
    http.addHeader("DOLAPIKEY", apiKey);

    int httpCode = http.POST(body);
    String response = "";

    if (httpCode > 0)
    {
        response = http.getString();
    }

    http.end();
    return response;
}

String DolibarrAPI::getProductByRef(String ref)
{
    String url = baseUrl + "/products?sqlfilters=(ref:=:" + ref + ")";
    return performGET(url);
}

String DolibarrAPI::getProductStock(int productId)
{
    String url = baseUrl + "/products/" + String(productId) + "/stock";
    return performGET(url);
}

String DolibarrAPI::createStockMovement(int productId, int warehouseId, int qty, String movementType)
{
    String url = baseUrl + "/stockmovements";

    String body = "{";
    body += "\"product_id\":" + String(productId) + ",";
    body += "\"warehouse_id\":" + String(warehouseId) + ",";
    body += "\"qty\":" + String(qty) + ",";
    body += "\"movement_type\":\"" + movementType + "\",";
    body += "\"label\":\"RFID Conveyor\"";
    body += "}";

    return performPOST(url, body);
}

String DolibarrAPI::pingDolibarr()
{
    String url = baseUrl + "/users/info";
    return performGET(url);
}

String DolibarrAPI::checkWarehouse(int warehouseId)
{
    String url = baseUrl + "/warehouses/" + String(warehouseId);
    return performGET(url);
}