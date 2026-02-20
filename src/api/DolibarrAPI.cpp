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

String DolibarrAPI::getProductByBarcode(String barcode)
{
    String url = baseUrl + "/products?sqlfilters=(barcode:=:" + barcode + ")";
    return performGET(url);
}

String DolibarrAPI::createStockMovement(int productId, int warehouseId, int qty)
{
    String url = baseUrl + "/stockmovements";

    String body = "{";
    body += "\"product_id\":" + String(productId) + ",";
    body += "\"warehouse_id\":" + String(warehouseId) + ",";
    body += "\"qty\":" + String(qty) + ",";
    body += "}";

    return performPOST(url, body);
}

boolean DolibarrAPI::checkConnection()
{
    if (WiFi.status() != WL_CONNECTED)
        return false;

    WiFiClient client;
    HTTPClient http;
    String url = baseUrl + "/users/info";

    Serial.print("URL: ");
    Serial.println(url);
    Serial.print("ESP32 IP: ");
    Serial.println(WiFi.localIP());
    Serial.print("Gateway: ");
    Serial.println(WiFi.gatewayIP());

    // http.setTimeout(8000);
    // Serial.print("http.begin: ");
    // Serial.print(http.begin(client, url));

    if (!http.begin(client, url))
        return false;

    Serial.print("apiKey: ");
    Serial.print(apiKey);

    http.addHeader("DOLAPIKEY", apiKey);
    int httpCode = http.GET();
    Serial.print(" http.GET(): ");
    Serial.print(httpCode);

    http.end();

    return httpCode == 200;
}

String DolibarrAPI::checkWarehouse(int warehouseId)
{
    String url = baseUrl + "/warehouses/" + String(warehouseId);
    return performGET(url);
}