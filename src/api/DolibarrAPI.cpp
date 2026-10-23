#include "dolibarr_api.h"

DolibarrAPI::DolibarrAPI(String url, String key)
{
    baseUrl = url;
    apiKey = key;
}

void DolibarrAPI::setTransport(IHttpClient *http, INetwork *network)
{
    _http = http;
    _network = network;
}

String DolibarrAPI::performGET(String url)
{
    HttpResponse response = _http->get(url, apiKey);
    return response.statusCode > 0 ? response.body : String("");
}

String DolibarrAPI::performPOST(String url, String body)
{
    HttpResponse response = _http->post(url, apiKey, body);
    return response.statusCode > 0 ? response.body : String("");
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
    body += "\"qty\":" + String(qty);
    body += "}";

    return performPOST(url, body);
}

boolean DolibarrAPI::checkConnection()
{
    if (!_network->isConnected())
        return false;

    String url = baseUrl + "/users/info";

    Serial.print("URL: ");
    Serial.println(url);
    Serial.print("ESP32 IP: ");
    Serial.println(_network->localIp());
    Serial.print("Gateway: ");
    Serial.println(_network->gatewayIp());

    HttpResponse response = _http->get(url, apiKey);
    Serial.print(" http.GET(): ");
    Serial.print(response.statusCode);

    return response.statusCode == 200;
}

String DolibarrAPI::checkWarehouse(int warehouseId)
{
    String url = baseUrl + "/warehouses/" + String(warehouseId);
    return performGET(url);
}
