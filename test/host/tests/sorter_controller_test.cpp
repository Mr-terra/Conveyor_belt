// SorterController tests: the device checks and one full sorting cycle.

#include <gmock/gmock.h>
#include <gtest/gtest.h>

#include "addresser.h"
#include "dolibarr_api.h"
#include "hardware_mocks.h"
#include "main_sorter.h"
#include "state_manager.h"

using ::testing::_;
using ::testing::NiceMock;
using ::testing::Return;

// Full sorting flow: RFID -> addresser (Dolibarr) -> servo.
class SorterControllerTest : public ::testing::Test
{
protected:
    void SetUp() override
    {
        ON_CALL(bus, probe(_)).WillByDefault(Return(0));
        ON_CALL(network, isConnected()).WillByDefault(Return(true));
        ON_CALL(rfid, isNewCardPresent()).WillByDefault(Return(false));

        sorter.setHardware(&rfid, &servo, &bus);
        api.setTransport(&http, &network);
        sorter.setDolibarrAPI(&api);
    }

    void TearDown() override
    {
        sorter.setDolibarrAPI(nullptr);
    }

    SorterController sorter;
    NiceMock<MockRfidDriver> rfid;
    NiceMock<MockServoDriver> servo;
    NiceMock<MockI2CBus> bus;
    NiceMock<MockHttpClient> http;
    NiceMock<MockNetwork> network;
    DolibarrAPI api{"http://dolibarr.local/api/index.php", "secret-key"};
};

TEST_F(SorterControllerTest, ChecksPassWhenRfidAndServoAcknowledge)
{
    EXPECT_TRUE(sorter.checkConnection());
}

TEST_F(SorterControllerTest, ChecksFailWhenRfidDoesNotAcknowledge)
{
    ON_CALL(bus, probe(0x28)).WillByDefault(Return(2));

    EXPECT_FALSE(sorter.checkConnection());
}

TEST_F(SorterControllerTest, ChecksFailWhenServoDoesNotAcknowledge)
{
    ON_CALL(bus, probe(0x38)).WillByDefault(Return(2));

    EXPECT_FALSE(sorter.checkConnection());
}

TEST_F(SorterControllerTest, InitializeStartsBothDrivers)
{
    StateManager stateManager;

    EXPECT_CALL(rfid, init()).Times(1);
    EXPECT_CALL(servo, begin()).Times(1);

    sorter.initialize(stateManager);
}

TEST_F(SorterControllerTest, ProductionWithoutCardDoesNotMoveServo)
{
    EXPECT_CALL(servo, writeAngle(_, _)).Times(0);

    sorter.production();
}

TEST_F(SorterControllerTest, ProductionSortsCardToItsWarehouseBin)
{
    ON_CALL(rfid, isNewCardPresent()).WillByDefault(Return(true));
    ON_CALL(rfid, readCardSerial()).WillByDefault(Return(true));
    ON_CALL(rfid, uidBytes(_, _))
        .WillByDefault([](uint8_t *buffer, uint8_t) {
            buffer[0] = 0x04;
            buffer[1] = 0x1A;
            buffer[2] = 0x2B;
            return uint8_t(3);
        });

    ON_CALL(http, get(_, _)).WillByDefault(Return(HttpResponse{
        200, String("[{\"id\":\"12\",\"array_options\":{"
                    "\"options_options_default_target_warehouse_id\":\"3\"}}]")}));
    EXPECT_CALL(http, post(_, _, _)).WillOnce(Return(HttpResponse{201, String("{}")}));

    EXPECT_CALL(servo, writeAngle(0, 60)).Times(1);

    sorter.production();
}

TEST_F(SorterControllerTest, ProductionWithUnknownCardSendsItToErrorBin)
{
    ON_CALL(rfid, isNewCardPresent()).WillByDefault(Return(true));
    ON_CALL(rfid, readCardSerial()).WillByDefault(Return(true));
    ON_CALL(rfid, uidBytes(_, _))
        .WillByDefault([](uint8_t *buffer, uint8_t) {
            buffer[0] = 0xDE;
            return uint8_t(1);
        });
    ON_CALL(http, get(_, _)).WillByDefault(Return(HttpResponse{200, String("[]")}));

    EXPECT_CALL(servo, writeAngle(0, 0)).Times(1); // error bin = BIN1

    sorter.production();
}
