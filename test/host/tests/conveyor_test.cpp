#include <gmock/gmock.h>
#include <gtest/gtest.h>

#include "conveyor.h"
#include "error.h"
#include "state_manager.h"
#include "hardware_mocks.h"

using ::testing::_;
using ::testing::NiceMock;
using ::testing::Return;

class ConveyorTest : public ::testing::Test
{
protected:
    void SetUp() override
    {
        hostMillis = 0;
        Error::clearCriticalErrors();
        Error::setErrorCallback(nullptr);
        conveyor.setHardware(&motor, &bus);
    }

    void TearDown() override
    {
        Error::clearCriticalErrors();
        Error::setErrorCallback(nullptr);
    }

    NiceMock<MockGrblMotor> motor;
    NiceMock<MockI2CBus> bus;
    Conveyor conveyor{10, 500};
};

TEST_F(ConveyorTest, StateToStringMapsKnownStatesAndFallsBackToUnknown)
{
    EXPECT_EQ(Conveyor::stateToString(CONVEYOR_STATE_STOPPED), String("STOPPED"));
    EXPECT_EQ(Conveyor::stateToString(CONVEYOR_STATE_RUNNING), String("RUNNING"));
    EXPECT_EQ(Conveyor::stateToString(CONVEYOR_STATE_ALARM), String("ALARM"));
    EXPECT_EQ(Conveyor::stateToString("garbage"), String("UNKNOWN"));
}

TEST_F(ConveyorTest, IdleStatusMapsToStopped)
{
    EXPECT_CALL(motor, readStatus()).WillOnce(Return(String("I")));

    EXPECT_EQ(conveyor.readHardwareState(), CONVEYOR_STATE_STOPPED);
    EXPECT_EQ(conveyor.getState(), CONVEYOR_STATE_STOPPED);
}

TEST_F(ConveyorTest, MovingStatusesMapToRunning)
{
    for (const char *code : {"R", "J", "H", "X"})
    {
        EXPECT_CALL(motor, readStatus()).WillOnce(Return(String(code)));
        EXPECT_EQ(conveyor.readHardwareState(), CONVEYOR_STATE_RUNNING) << "code " << code;
    }
}

TEST_F(ConveyorTest, AlarmStatusMapsToAlarm)
{
    EXPECT_CALL(motor, readStatus()).WillOnce(Return(String("A")));

    EXPECT_EQ(conveyor.readHardwareState(), CONVEYOR_STATE_ALARM);
}

TEST_F(ConveyorTest, EmptyStatusFallsBackToLockAndIdleHelpers)
{
    EXPECT_CALL(motor, readStatus()).WillOnce(Return(String("")));
    EXPECT_CALL(motor, inLock()).WillOnce(Return(true));
    EXPECT_EQ(conveyor.readHardwareState(), CONVEYOR_STATE_ALARM);

    EXPECT_CALL(motor, readStatus()).WillOnce(Return(String("")));
    EXPECT_CALL(motor, inLock()).WillOnce(Return(false));
    EXPECT_CALL(motor, readIdle()).WillOnce(Return(true));
    EXPECT_EQ(conveyor.readHardwareState(), CONVEYOR_STATE_STOPPED);

    EXPECT_CALL(motor, readStatus()).WillOnce(Return(String("")));
    EXPECT_CALL(motor, inLock()).WillOnce(Return(false));
    EXPECT_CALL(motor, readIdle()).WillOnce(Return(false));
    EXPECT_EQ(conveyor.readHardwareState(), CONVEYOR_STATE_RUNNING);
}

TEST_F(ConveyorTest, MonitorHardwareReadsAtMostEvery100Milliseconds)
{
    EXPECT_CALL(motor, readStatus()).Times(2).WillRepeatedly(Return(String("I")));

    hostMillis = 1000;
    conveyor.monitorHardware(); // reads
    hostMillis = 1050;
    conveyor.monitorHardware(); // throttled
    hostMillis = 1200;
    conveyor.monitorHardware(); // reads again
}

TEST_F(ConveyorTest, StartUnlocksLockedModule)
{
    EXPECT_CALL(motor, inLock()).WillOnce(Return(true));
    EXPECT_CALL(motor, unLock()).Times(1);

    conveyor.start();
}

TEST_F(ConveyorTest, StartDoesNotUnlockUnlockedModule)
{
    EXPECT_CALL(motor, inLock()).WillOnce(Return(false));
    EXPECT_CALL(motor, unLock()).Times(0);

    conveyor.start();
}

TEST_F(ConveyorTest, StopSendsZeroMotorCommandAndWaitsIdle)
{
    EXPECT_CALL(motor, setMotor(0, 0, 0, 0)).Times(1);
    EXPECT_CALL(motor, waitIdle()).Times(1);

    conveyor.stop();
}

TEST_F(ConveyorTest, MoveForwardUsesMovementAndSpeed)
{
    EXPECT_CALL(motor, setMotor(10, 0, 0, 500)).Times(1);

    conveyor.move(true);
}

TEST_F(ConveyorTest, MoveBackwardInvertsMovement)
{
    EXPECT_CALL(motor, setMotor(-10, 0, 0, 500)).Times(1);

    conveyor.move(false);
}

TEST_F(ConveyorTest, MoveStartsMotorWhenNotRunning)
{
    EXPECT_CALL(motor, inLock()).WillOnce(Return(true));
    EXPECT_CALL(motor, unLock()).Times(1);

    conveyor.move(true);
}

TEST_F(ConveyorTest, SpeedIsClampedToValidRange)
{
    Conveyor fast(5, 5000);
    fast.setHardware(&motor, &bus);
    EXPECT_CALL(motor, setMotor(5, 0, 0, 1000)).Times(1);
    fast.move(true);

    conveyor.setSpeed(2000);
    EXPECT_CALL(motor, setMotor(10, 0, 0, 1000)).Times(1);
    conveyor.move(true);

    conveyor.setSpeed(-3);
    EXPECT_CALL(motor, setMotor(10, 0, 0, 0)).Times(1);
    conveyor.move(true);
}

TEST_F(ConveyorTest, SetMovementChangesStepSize)
{
    conveyor.setMovement(4);

    EXPECT_CALL(motor, setMotor(4, 0, 0, 500)).Times(1);
    conveyor.move(true);
}

TEST_F(ConveyorTest, DeprecatedSetStateIsIgnored)
{
    EXPECT_CALL(motor, readStatus()).WillOnce(Return(String("I")));
    conveyor.readHardwareState();

    conveyor.setState(CONVEYOR_STATE_ALARM);

    EXPECT_EQ(conveyor.getState(), CONVEYOR_STATE_STOPPED);
}

TEST_F(ConveyorTest, CheckConnectionFailsWhenAddressDoesNotAcknowledge)
{
    EXPECT_CALL(bus, probe(0x70)).WillOnce(Return(2));
    EXPECT_CALL(motor, readStatus()).Times(0);

    EXPECT_FALSE(conveyor.checkConnection());
}

TEST_F(ConveyorTest, CheckConnectionFailsWhenModuleDoesNotAnswer)
{
    EXPECT_CALL(bus, probe(0x70)).WillOnce(Return(0));
    EXPECT_CALL(motor, readStatus()).WillOnce(Return(String("")));

    EXPECT_FALSE(conveyor.checkConnection());
}

TEST_F(ConveyorTest, CheckConnectionFailsWhenModuleIsLocked)
{
    EXPECT_CALL(bus, probe(0x70)).WillOnce(Return(0));
    EXPECT_CALL(motor, readStatus()).WillOnce(Return(String("I")));
    EXPECT_CALL(motor, inLock()).WillOnce(Return(true));

    EXPECT_FALSE(conveyor.checkConnection());
}

TEST_F(ConveyorTest, CheckConnectionSucceedsWhenModuleIsOperational)
{
    EXPECT_CALL(bus, probe(0x70)).WillOnce(Return(0));
    EXPECT_CALL(motor, readStatus()).WillOnce(Return(String("I")));
    EXPECT_CALL(motor, inLock()).WillOnce(Return(false));

    EXPECT_TRUE(conveyor.checkConnection());
}

TEST_F(ConveyorTest, InitializeMovesToAlarmWhenHardwareReportsAlarm)
{
    StateManager stateManager;
    std::vector<String> messages;
    Error::setErrorCallback([&](const Error &error) { messages.push_back(error.getMessage()); });

    EXPECT_CALL(motor, readStatus()).WillOnce(Return(String("I")));
    conveyor.initialize(stateManager);
    ASSERT_EQ(conveyor.getState(), CONVEYOR_STATE_STOPPED);

    EXPECT_CALL(motor, readStatus()).WillOnce(Return(String("A")));
    conveyor.readHardwareState();
    stateManager.monitorAllStates();

    EXPECT_TRUE(Error::hasCriticalErrors());
    bool sawAlarmRule = false;
    for (const String &message : messages)
    {
        if (message == String("Hardware alarm while stopped"))
            sawAlarmRule = true;
    }
    EXPECT_TRUE(sawAlarmRule);
}
