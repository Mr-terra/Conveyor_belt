// DolibarrAPI: builds the Dolibarr REST requests and sends them through the injected transport.
// The product lookup returns JSON text; the caller parses it (see Addresser).

#include "dolibarr_api.h"
#include "log.h"

// Stores the server URL and the API key. Nothing is sent until a transport is set.
DolibarrAPI::DolibarrAPI(String url, String key)
{
    baseUrl = url;
    apiKey = key;
}

// Injects the HTTP client and the network status. Without them no request can be sent.
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

// The JSON body is built by hand: product_id, warehouse_id and qty.
bool DolibarrAPI::createStockMovement(int productId, int warehouseId, int qty)
{
    String url = baseUrl + "/stockmovements";

    String body = "{";
    body += "\"product_id\":" + String(productId) + ",";
    body += "\"warehouse_id\":" + String(warehouseId) + ",";
    body += "\"qty\":" + String(qty);
    body += "}";

    HttpResponse response = _http->post(url, apiKey, body);
    return response.statusCode == 200 || response.statusCode == 201;
}

// Returns false without sending anything when Wi-Fi is down.
boolean DolibarrAPI::checkConnection()
{
    if (!_network->isConnected())
        return false;

    String url = baseUrl + "/users/info";

    HttpResponse response = _http->get(url, apiKey);
    if (response.statusCode != 200)
        LOG_WARN("Dolibarr", "HTTP %d from /users/info", response.statusCode);

    return response.statusCode == 200;
}
