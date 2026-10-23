#include <ArduinoJson.h>
#include <gmock/gmock.h>
#include <gtest/gtest.h>

#include "dolibarr_api.h"
#include "hardware_mocks.h"

using ::testing::_;
using ::testing::NiceMock;
using ::testing::Return;

class DolibarrApiTest : public ::testing::Test
{
protected:
    void SetUp() override
    {
        ON_CALL(network, isConnected()).WillByDefault(Return(true));
        ON_CALL(network, localIp()).WillByDefault(Return(String("192.168.1.50")));
        ON_CALL(network, gatewayIp()).WillByDefault(Return(String("192.168.1.1")));
        api.setTransport(&http, &network);
    }

    NiceMock<MockHttpClient> http;
    NiceMock<MockNetwork> network;
    DolibarrAPI api{"http://dolibarr.local/api/index.php", "secret-key"};
};

TEST_F(DolibarrApiTest, GetProductByBarcodeQueriesProductsWithSqlFilter)
{
    EXPECT_CALL(http, get(String("http://dolibarr.local/api/index.php/products?sqlfilters=(barcode:=:'041A2B')"),
                          String("secret-key")))
        .WillOnce(Return(HttpResponse{200, String("[]")}));

    EXPECT_EQ(api.getProductByBarcode("'041A2B'"), String("[]"));
}

TEST_F(DolibarrApiTest, TransportFailureReturnsEmptyBody)
{
    ON_CALL(http, get(_, _)).WillByDefault(Return(HttpResponse{-1, String("ignored")}));

    EXPECT_EQ(api.checkWarehouse(3), String(""));
}

TEST_F(DolibarrApiTest, CheckWarehouseQueriesWarehouseById)
{
    EXPECT_CALL(http, get(String("http://dolibarr.local/api/index.php/warehouses/3"), String("secret-key")))
        .WillOnce(Return(HttpResponse{200, String("{\"id\":3}")}));

    EXPECT_EQ(api.checkWarehouse(3), String("{\"id\":3}"));
}

TEST_F(DolibarrApiTest, CreateStockMovementSendsValidJson)
{
    String sentBody;
    EXPECT_CALL(http, post(String("http://dolibarr.local/api/index.php/stockmovements"), String("secret-key"), _))
        .WillOnce([&](const String &, const String &, const String &body) {
            sentBody = body;
            return HttpResponse{200, String("{}")};
        });

    api.createStockMovement(12, 2, 1);

    DynamicJsonDocument doc(512);
    DeserializationError error = deserializeJson(doc, sentBody.c_str());
    ASSERT_FALSE(error) << "body was: " << sentBody.c_str();
    EXPECT_EQ(doc["product_id"].as<int>(), 12);
    EXPECT_EQ(doc["warehouse_id"].as<int>(), 2);
    EXPECT_EQ(doc["qty"].as<int>(), 1);
}

TEST_F(DolibarrApiTest, CheckConnectionIsFalseWithoutWifi)
{
    ON_CALL(network, isConnected()).WillByDefault(Return(false));

    EXPECT_CALL(http, get(_, _)).Times(0);
    EXPECT_FALSE(api.checkConnection());
}

TEST_F(DolibarrApiTest, CheckConnectionIsTrueOnHttp200)
{
    EXPECT_CALL(http, get(String("http://dolibarr.local/api/index.php/users/info"), String("secret-key")))
        .WillOnce(Return(HttpResponse{200, String("{}")}));

    EXPECT_TRUE(api.checkConnection());
}

TEST_F(DolibarrApiTest, CheckConnectionIsFalseOnAuthenticationError)
{
    ON_CALL(http, get(_, _)).WillByDefault(Return(HttpResponse{403, String("")}));

    EXPECT_FALSE(api.checkConnection());
}
