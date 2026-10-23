// CheckConnect tests: each device check passes or fails as expected,
// and a Dolibarr error fails the whole check.

#include <gmock/gmock.h>
#include <gtest/gtest.h>

#include "check_connect.h"
#include "conveyor.h"
#include "dolibarr_api.h"
#include "hardware_mocks.h"
#include "main_sorter.h"

using ::testing::_;
using ::testing::NiceMock;
using ::testing::Return;

class CheckConnectTest : public ::testing::Test
{
protected:
    void SetUp() override
    {
        ON_CALL(motor, readStatus()).WillByDefault(Return(String("I")));
        ON_CALL(bus, probe(_)).WillByDefault(Return(0));
        ON_CALL(network, isConnected()).WillByDefault(Return(true));
        ON_CALL(http, get(_, _)).WillByDefault(Return(HttpResponse{200, String("{}")}));

        conveyor.setHardware(&motor, &bus);
        sorterController.setHardware(&rfid, &servo, &bus);
        api.setTransport(&http, &network);
    }

    NiceMock<MockGrblMotor> motor;
    NiceMock<MockI2CBus> bus;
    NiceMock<MockRfidDriver> rfid;
    NiceMock<MockServoDriver> servo;
    NiceMock<MockHttpClient> http;
    NiceMock<MockNetwork> network;

    Conveyor conveyor{10, 500};
    SorterController sorterController;
    DolibarrAPI api{"http://localhost/api/index.php", "test-key"};
    CheckConnect checkConnect;
};

TEST_F(CheckConnectTest, ChecksAbortWhenDependenciesAreMissing)
{
    EXPECT_FALSE(checkConnect.areDependenciesConfigured());
    EXPECT_FALSE(checkConnect.checkConnections());
}

TEST_F(CheckConnectTest, AllDevicesHealthyPassesChecks)
{
    checkConnect.setDolibarrAPI(&api);
    checkConnect.setConveyor(&conveyor);
    checkConnect.setSorterController(&sorterController);

    EXPECT_TRUE(checkConnect.areDependenciesConfigured());
    EXPECT_TRUE(checkConnect.checkConnections());
}

TEST_F(CheckConnectTest, DolibarrErrorFailsChecks)
{
    ON_CALL(http, get(_, _)).WillByDefault(Return(HttpResponse{500, String()}));
    checkConnect.setDolibarrAPI(&api);
    checkConnect.setConveyor(&conveyor);
    checkConnect.setSorterController(&sorterController);

    EXPECT_FALSE(checkConnect.checkConnections());
}

TEST_F(CheckConnectTest, GrblNotAcknowledgingFailsMotorCheck)
{
    checkConnect.setDolibarrAPI(&api);
    checkConnect.setConveyor(&conveyor);
    checkConnect.setSorterController(&sorterController);
    ON_CALL(bus, probe(0x70)).WillByDefault(Return(2));

    EXPECT_FALSE(checkConnect.checkMotorConnection());
    EXPECT_FALSE(checkConnect.checkConnections());
}

TEST_F(CheckConnectTest, RfidNotAcknowledgingFailsSorterCheck)
{
    checkConnect.setDolibarrAPI(&api);
    checkConnect.setConveyor(&conveyor);
    checkConnect.setSorterController(&sorterController);
    ON_CALL(bus, probe(0x28)).WillByDefault(Return(2));

    EXPECT_FALSE(checkConnect.checkSorterConnection());
    EXPECT_FALSE(checkConnect.checkConnections());
}

TEST_F(CheckConnectTest, ServoNotAcknowledgingFailsSorterCheck)
{
    checkConnect.setDolibarrAPI(&api);
    checkConnect.setConveyor(&conveyor);
    checkConnect.setSorterController(&sorterController);
    ON_CALL(bus, probe(0x38)).WillByDefault(Return(2));

    EXPECT_FALSE(checkConnect.checkSorterConnection());
}

TEST_F(CheckConnectTest, DolibarrReachableWhenWifiUpAndApiAnswers200)
{
    checkConnect.setDolibarrAPI(&api);

    EXPECT_TRUE(checkConnect.checkDolibarrConnection());
}

TEST_F(CheckConnectTest, DolibarrUnreachableWithoutWifi)
{
    checkConnect.setDolibarrAPI(&api);
    ON_CALL(network, isConnected()).WillByDefault(Return(false));

    EXPECT_CALL(http, get(_, _)).Times(0);
    EXPECT_FALSE(checkConnect.checkDolibarrConnection());
}

TEST_F(CheckConnectTest, DolibarrErrorStatusIsReportedAsFailure)
{
    checkConnect.setDolibarrAPI(&api);
    ON_CALL(http, get(_, _)).WillByDefault(Return(HttpResponse{401, String("")}));

    EXPECT_FALSE(checkConnect.checkDolibarrConnection());
}

TEST_F(CheckConnectTest, UnconfiguredDolibarrIsSkipped)
{
    checkConnect.setDolibarrAPI(nullptr);

    EXPECT_TRUE(checkConnect.checkDolibarrConnection());
}
