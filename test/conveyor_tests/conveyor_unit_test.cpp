#include <gtest/gtest.h>
#include <gmock/gmock.h>
#include "conveyor.h"
#include "mock_grbl.h"
#include "mock_wire.h"

using ::testing::Return;
using ::testing::NiceMock;
using ::testing::_;

// =============================================
// TESTS FOR THE checkConnection() METHOD
// =============================================

TEST(ConveyorUnitTest, CheckConnection_Success_AllChecksPass) {
    // Arrange
    NiceMock<MockWire> mockWire;
    NiceMock<MockGRBL> mockGrbl;
    Conveyor conveyor(10, 500);
    conveyor._grbl = &mockGrbl;

    // Configure the mock for a full success
    EXPECT_CALL(mockWire, beginTransmission(CONVEYOR_ADDR))
        .WillOnce(Return(0)); // I2C success
    EXPECT_CALL(mockGrbl, readStatus())
        .WillOnce(Return("Idle")); // Valid response
    EXPECT_CALL(mockGrbl, inLock())
        .WillOnce(Return(false)); // Not locked

    // Act
    bool result = conveyor.checkConnection();

    // Assert
    EXPECT_TRUE(result);
}

// =============================================
// TESTS FOR THE readHardwareState() METHOD
// =============================================

TEST(ConveyorUnitTest, ReadHardwareState_IdleStatus_ReturnsStopped) {
    // Arrange
    NiceMock<MockGRBL> mockGrbl;
    Conveyor conveyor(10, 500);
    conveyor._grbl = &mockGrbl;

    // Configure the mock for an Idle state
    EXPECT_CALL(mockGrbl, readStatus())
        .WillOnce(Return("I")); // 'I' for Idle

    // Act
    String result = conveyor.readHardwareState();

    // Assert
    EXPECT_EQ("STOPPED", result);
    EXPECT_EQ("STOPPED", conveyor.getState());
}