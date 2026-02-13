#ifndef DOLIBARR_API_H
#define DOLIBARR_API_H

#include <Arduino.h>
#include <HTTPClient.h>
#include <WiFi.h>

class DolibarrAPI
{
private:
   String baseUrl;
   String apiKey;

   String performGET(String url);
   String performPOST(String url, String body);
public:
   DolibarrAPI(String url, String key);

   // Produit
   String getProductByRef(String ref);
   String getProductStock(int productId);

   // Mouvement
   String createStockMovement(int productId, int warehouseId, int qty, String movementType);

   // Vérifications système
   bool checkConnection();
   String checkWarehouse(int warehouseId);
};

#endif
