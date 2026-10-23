#ifndef DOLIBARR_API_H
#define DOLIBARR_API_H

#include <Arduino.h>
#include <HTTPClient.h>

class DolibarrAPI
{
private:
    String baseUrl;
    String apiKey;

    String performGET(String url);
    String performPOST(String url, String body);

public:
    // Constructor
    DolibarrAPI(String url, String key);

    // API connection check
    boolean checkConnection();

    // Produits
    String getProductByBarcode(String barcode);

    // Stock
    String createStockMovement(int productId, int warehouseId, int qty);

    // Warehouses
    String checkWarehouse(int warehouseId);
};

#endif
