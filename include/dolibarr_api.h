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
    // Constructeur
    DolibarrAPI(String url, String key);

    // Vérification connexion API
    boolean checkConnection();

    // Produits
    String getProductByBarcode(String barcode);

    // Stock
    String createStockMovement(int productId, int warehouseId, int qty);

    // Entrepôts
    String checkWarehouse(int warehouseId);
};

#endif
