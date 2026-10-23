// Scheduler tests: the real scheduler and modules run with hardware doubles.
// They cover the normal sequence, the failure paths, and recovery from ERROR.

#include <gmock/gmock.h>
#include <gtest/gtest.h>

#include "dolibarr_api.h"
#include "check_connect.h"
#include "conveyor.h"
#include "error.h"
#include "init_position.h"
#include "main_sorter.h"
#include "scheduler.h"
#include "hardware_mocks.h"

using ::testing::_;
using ::testing::NiceMock;
using ::testing::Return;

// Runs the real scheduler against the real modules, with every hardware
// access replaced by a gMock double.
class SchedulerTest : public ::testing::Test
{
protected:
    void SetUp() override
    {
        hostMillis = 0;
        Error::clearCriticalErrors();
        Error::setErrorCallback(nullptr);

        // Healthy hardware by default: GRBL answers "Idle", every I2C device acknowledges.
        ON_CALL(motor, readStatus()).WillByDefault(Return(String("I")));
        ON_CALL(bus, probe(_)).WillByDefault(Return(0));
        ON_CALL(network, isConnected()).WillByDefault(Return(true));
        ON_CALL(http, get(_, _)).WillByDefault(Return(HttpResponse{200, String("{}")}));

        conveyor.setHardware(&motor, &bus);
        sorterController.setHardware(&rfid, &servo, &bus);
        api.setTransport(&http, &network);

        checkConnect.setDolibarrAPI(&api);
        checkConnect.setConveyor(&conveyor);
        checkConnect.setSorterController(&sorterController);

        scheduler.initialize();
        scheduler.setDependencies(&checkConnect, &initPosition, &sorterController, &conveyor);
    }

    void TearDown() override
    {
        Error::clearCriticalErrors();
        Error::setErrorCallback(nullptr);
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
    InitPosition initPosition;
    Scheduler scheduler;
};

TEST_F(SchedulerTest, InitializeStartsInWaiting)
{
    EXPECT_EQ(scheduler.getState(), SchedulerState::Waiting);
}

TEST_F(SchedulerTest, TriggerCheckConnectMovesToCheckConnect)
{
    scheduler.triggerCheckConnect();

    EXPECT_EQ(scheduler.getState(), SchedulerState::CheckConnect);
}

TEST_F(SchedulerTest, TriggerCheckConnectWithoutDependenciesGoesToError)
{
    scheduler.setDependencies(nullptr, &initPosition, &sorterController, &conveyor);

    scheduler.triggerCheckConnect();

    EXPECT_EQ(scheduler.getState(), SchedulerState::Error);
}

TEST_F(SchedulerTest, SuccessfulCheckConnectMovesToInitPositions)
{
    scheduler.setState(SchedulerState::CheckConnect);

    scheduler.executeCurrentState();

    EXPECT_EQ(scheduler.getState(), SchedulerState::InitPositions);
}

TEST_F(SchedulerTest, FailedCheckConnectReturnsToWaiting)
{
    ON_CALL(bus, probe(0x70)).WillByDefault(Return(2)); // GRBL not acknowledging
    scheduler.setState(SchedulerState::CheckConnect);

    scheduler.executeCurrentState();

    EXPECT_EQ(scheduler.getState(), SchedulerState::Waiting);
}

TEST_F(SchedulerTest, InitPositionsMovesToProduction)
{
    scheduler.setState(SchedulerState::InitPositions);

    scheduler.executeCurrentState();

    EXPECT_EQ(scheduler.getState(), SchedulerState::Production);
}

TEST_F(SchedulerTest, ProductionMovesConveyorForward)
{
    scheduler.setState(SchedulerState::Production);

    EXPECT_CALL(motor, setMotor(10, 0, 0, 500)).Times(1);
    scheduler.executeCurrentState();
}

TEST_F(SchedulerTest, ProductionStopsWhenConveyorIsInAlarm)
{
    ON_CALL(motor, readStatus()).WillByDefault(Return(String("A")));
    conveyor.readHardwareState();
    scheduler.setState(SchedulerState::Production);

    EXPECT_CALL(motor, setMotor(_, _, _, _)).Times(0);
    scheduler.executeCurrentState();

    EXPECT_EQ(scheduler.getState(), SchedulerState::Waiting);
}

TEST_F(SchedulerTest, ErrorStateRecoversWhenConditionsAreMet)
{
    scheduler.setState(SchedulerState::Error);

    scheduler.executeCurrentState();

    EXPECT_EQ(scheduler.getState(), SchedulerState::Waiting);
}

TEST_F(SchedulerTest, ErrorStateStaysWhenConveyorIsStillInAlarm)
{
    ON_CALL(motor, readStatus()).WillByDefault(Return(String("A")));
    conveyor.readHardwareState();
    scheduler.setState(SchedulerState::Error);

    scheduler.executeCurrentState();

    EXPECT_EQ(scheduler.getState(), SchedulerState::Error);
}

TEST_F(SchedulerTest, LatchedCriticalErrorBlocksRecoveryUntilCleared)
{
    Error::triggerError("overheat", ERROR_CRITICAL, "Conveyor");
    scheduler.setState(SchedulerState::Error);

    scheduler.executeCurrentState();
    EXPECT_EQ(scheduler.getState(), SchedulerState::Error);

    Error::clearCriticalErrors();
    scheduler.executeCurrentState();
    EXPECT_EQ(scheduler.getState(), SchedulerState::Waiting);
    EXPECT_FALSE(Error::hasCriticalErrors());
}

TEST_F(SchedulerTest, ErrorStateStaysWhenConnectionsAreBroken)
{
    ON_CALL(bus, probe(0x70)).WillByDefault(Return(2));
    scheduler.setState(SchedulerState::Error);

    scheduler.executeCurrentState();

    EXPECT_EQ(scheduler.getState(), SchedulerState::Error);
}

TEST_F(SchedulerTest, TriggerStopReturnsToWaitingAndStopsConveyor)
{
    scheduler.setState(SchedulerState::Production);

    EXPECT_CALL(motor, setMotor(0, 0, 0, 0)).Times(1);
    scheduler.triggerStop();

    EXPECT_EQ(scheduler.getState(), SchedulerState::Waiting);
}

TEST_F(SchedulerTest, WaitingDoesNotDriveHardware)
{
    EXPECT_CALL(motor, setMotor(_, _, _, _)).Times(0);
    EXPECT_CALL(servo, writeAngle(_, _)).Times(0);

    scheduler.executeCurrentState();

    EXPECT_EQ(scheduler.getState(), SchedulerState::Waiting);
}
