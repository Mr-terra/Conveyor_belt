#include <gmock/gmock.h>
#include <gtest/gtest.h>

#include "hardware_mocks.h"
#include "servo_sorter.h"
#include "state_manager.h"

using ::testing::_;
using ::testing::InSequence;
using ::testing::NiceMock;
using ::testing::Return;

class ServoSorterTest : public ::testing::Test
{
protected:
    void SetUp() override
    {
        ON_CALL(bus, probe(_)).WillByDefault(Return(0));
        sorter.setHardware(&servo, &bus);
    }

    NiceMock<MockServoDriver> servo;
    NiceMock<MockI2CBus> bus;
    ServoSorter sorter;
};

TEST_F(ServoSorterTest, InitStartsDriverAndParksAtNeutralAngle)
{
    StateManager stateManager;

    InSequence order;
    EXPECT_CALL(servo, begin()).Times(1);
    EXPECT_CALL(servo, writeAngle(0, 30)).Times(1);

    sorter.init(stateManager);
}

TEST_F(ServoSorterTest, Bin1PointsToZeroDegrees)
{
    EXPECT_CALL(servo, writeAngle(0, 0)).Times(1);

    sorter.moveTo(Bin::BIN1);
}

TEST_F(ServoSorterTest, Bin2PointsTo30Degrees)
{
    EXPECT_CALL(servo, writeAngle(0, 30)).Times(1);

    sorter.moveTo(Bin::BIN2);
}

TEST_F(ServoSorterTest, Bin3PointsTo60Degrees)
{
    EXPECT_CALL(servo, writeAngle(0, 60)).Times(1);

    sorter.moveTo(Bin::BIN3);
}

TEST_F(ServoSorterTest, UnknownBinLeavesServoInPlace)
{
    EXPECT_CALL(servo, writeAngle(_, _)).Times(0);

    sorter.moveTo(Bin::NONE);
}

TEST_F(ServoSorterTest, CheckConnectionUsesGoPlusI2cAddress)
{
    EXPECT_CALL(bus, probe(0x38)).WillOnce(Return(0)).WillOnce(Return(2));
    EXPECT_TRUE(sorter.checkConnection());
    EXPECT_FALSE(sorter.checkConnection());
}
