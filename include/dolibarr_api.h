#ifndef DOLIBARR_API_H
#define DOLIBARR_API_H

#include <Arduino.h>
#include "hal/http_client.h"
#include "hal/network.h"

class DolibarrAPI
{
private:
    String baseUrl;
    String apiKey;
    IHttpClient *_http = nullptr;
    INetwork *_network = nullptr;

    String performGET(String url);
    String performPOST(String url, String body);

public:
    // Constructor
    DolibarrAPI(String url, String key);

    // Injects the HTTP transport and the network link. Must be called before use.
    void setTransport(IHttpClient *http, INetwork *network);

    // API connection check
    boolean checkConnection();

    // Products
    String getProductByBarcode(String barcode);

    // Stock
    String createStockMovement(int productId, int warehouseId, int qty);
};

#endif
