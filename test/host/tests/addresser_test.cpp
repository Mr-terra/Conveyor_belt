#include <gmock/gmock.h>
#include <gtest/gtest.h>

#include "addresser.h"
#include "dolibarr_api.h"
#include "hardware_mocks.h"

using ::testing::_;
using ::testing::NiceMock;
using ::testing::Return;

class AddresserTest : public ::testing::Test
{
protected:
    void SetUp() override
    {
        ON_CALL(network, isConnected()).WillByDefault(Return(true));
        api.setTransport(&http, &network);
        addresser.setDolibarrAPI(&api);
    }

    // Product lookup answer: one product in the given warehouse.
    static HttpResponse productIn(int warehouseId)
    {
        return HttpResponse{200, String("[{\"id\":\"12\",\"array_options\":{"
                                        "\"options_options_default_target_warehouse_id\":\"" +
                                        String(warehouseId) + "\"}}]")};
    }

    NiceMock<MockHttpClient> http;
    NiceMock<MockNetwork> network;
    DolibarrAPI api{"http://dolibarr.local/api/index.php", "secret-key"};
    Addresser addresser;
};

TEST_F(AddresserTest, ReturnsNoneWhenDolibarrIsNotConfigured)
{
    Addresser unconfigured;

    EXPECT_EQ(unconfigured.resolve("04:1A:2B"), Bin::NONE);
}

TEST_F(AddresserTest, StripsColonsFromUidBeforeQuerying)
{
    EXPECT_CALL(http, get(String("http://dolibarr.local/api/index.php/products?sqlfilters=(barcode:=:'041A2B')"), _))
        .WillOnce(Return(HttpResponse{200, String("[]")}));

    addresser.resolve("04:1A:2B");
}

TEST_F(AddresserTest, ProductInWarehouse1GoesToBin1AndRecordsStockMovement)
{
    ON_CALL(http, get(_, _)).WillByDefault(Return(productIn(1)));

    EXPECT_CALL(http, post(_, _, String("{\"product_id\":12,\"warehouse_id\":1,\"qty\":1}")))
        .WillOnce(Return(HttpResponse{201, String("{}")}));

    EXPECT_EQ(addresser.resolve("04:1A:2B"), Bin::BIN1);
}

TEST_F(AddresserTest, WarehouseIdSelectsMatchingBin)
{
    ON_CALL(http, post(_, _, _)).WillByDefault(Return(HttpResponse{201, String("{}")}));

    ON_CALL(http, get(_, _)).WillByDefault(Return(productIn(2)));
    EXPECT_EQ(addresser.resolve("04:1A:2B"), Bin::BIN2);

    ON_CALL(http, get(_, _)).WillByDefault(Return(productIn(3)));
    EXPECT_EQ(addresser.resolve("04:1A:2B"), Bin::BIN3);
}

TEST_F(AddresserTest, UnknownWarehouseIsNotSortedAndNoStockMovementIsRecorded)
{
    ON_CALL(http, get(_, _)).WillByDefault(Return(productIn(7)));

    EXPECT_CALL(http, post(_, _, _)).Times(0);
    EXPECT_EQ(addresser.resolve("04:1A:2B"), Bin::NONE);
}

TEST_F(AddresserTest, EmptyResponseReturnsNone)
{
    ON_CALL(http, get(_, _)).WillByDefault(Return(HttpResponse{-1, String("")}));

    EXPECT_EQ(addresser.resolve("04:1A:2B"), Bin::NONE);
}

TEST_F(AddresserTest, MalformedJsonReturnsNone)
{
    ON_CALL(http, get(_, _)).WillByDefault(Return(HttpResponse{200, String("[{\"id\":")}));

    EXPECT_CALL(http, post(_, _, _)).Times(0);
    EXPECT_EQ(addresser.resolve("04:1A:2B"), Bin::NONE);
}

TEST_F(AddresserTest, NonArrayJsonReturnsNone)
{
    ON_CALL(http, get(_, _)).WillByDefault(Return(HttpResponse{200, String("{\"id\":\"12\"}")}));

    EXPECT_EQ(addresser.resolve("04:1A:2B"), Bin::NONE);
}
