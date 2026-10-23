// DolibarrAPI: REST client for the Dolibarr API.

#ifndef DOLIBARR_API_H
#define DOLIBARR_API_H

#include <Arduino.h>
#include "hal/http_client.h"
#include "hal/network.h"

// REST client for the Dolibarr API. The HTTP transport is injected with setTransport(),
// so the logic stays independent of the ESP32 libraries.
class DolibarrAPI
{
private:
    // Server URL, for example http://host/api/index.php.
    String baseUrl;
    // Sent in the DOLAPIKEY header of every request.
    String apiKey;
    IHttpClient *_http = nullptr;
    INetwork *_network = nullptr;

    // Sends a GET request. Returns the response body, or an empty string if the request failed.
    String performGET(String url);
    // Sends a POST request. Returns the response body, or an empty string if the request failed.
    String performPOST(String url, String body);

public:
    // Constructor
    DolibarrAPI(String url, String key);

    // Injects the HTTP transport and the network link. Must be called before use.
    void setTransport(IHttpClient *http, INetwork *network);

    // API connection check
    // Returns true when the API answers HTTP 200 on /users/info. Needs Wi-Fi.
    boolean checkConnection();

    // Products
    // Looks products up by barcode. Returns the JSON array text, or an empty string on failure.
    String getProductByBarcode(String barcode);

    // Stock
    // Records a stock movement: qty units of the product leave the warehouse.
    bool createStockMovement(int productId, int warehouseId, int qty);  // true on HTTP 200/201
};

#endif
