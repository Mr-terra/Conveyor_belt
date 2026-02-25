#include <gtest/gtest.h>
#include <gmock/gmock.h>
#include "conveyor.h"
#include "mock_grbl.h"
#include "mock_wire.h"

using ::testing::Return;
using ::testing::NiceMock;
using ::testing::_;

// =============================================
// TESTS DE LA MÉTHODE checkConnection()
// =============================================

TEST(ConveyorUnitTest, CheckConnection_Success_AllChecksPass) {
    // Arrange
    NiceMock<MockWire> mockWire;
    NiceMock<MockGRBL> mockGrbl;
    Conveyor conveyor(10, 500);
    conveyor._grbl = &mockGrbl;

    // Configurer le mock pour un succès complet
    EXPECT_CALL(mockWire, beginTransmission(CONVEYOR_ADDR))
        .WillOnce(Return(0)); // I2C succès
    EXPECT_CALL(mockGrbl, readStatus())
        .WillOnce(Return("Idle")); // Réponse valide
    EXPECT_CALL(mockGrbl, inLock())
        .WillOnce(Return(false)); // Pas verrouillé

    // Act
    bool result = conveyor.checkConnection();

    // Assert
    EXPECT_TRUE(result);
}

// =============================================
// TESTS DE LA MÉTHODE readHardwareState()
// =============================================

TEST(ConveyorUnitTest, ReadHardwareState_IdleStatus_ReturnsStopped) {
    // Arrange
    NiceMock<MockGRBL> mockGrbl;
    Conveyor conveyor(10, 500);
    conveyor._grbl = &mockGrbl;

    // Configurer le mock pour un état Idle
    EXPECT_CALL(mockGrbl, readStatus())
        .WillOnce(Return("I")); // 'I' pour Idle

    // Act
    String result = conveyor.readHardwareState();

    // Assert
    EXPECT_EQ("STOPPED", result);
    EXPECT_EQ("STOPPED", conveyor.getState());
}