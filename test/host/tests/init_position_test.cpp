#include <gtest/gtest.h>

#include "init_position.h"

TEST(InitPositionTest, InitializePositionsSucceedsWhenBothStepsSucceed)
{
    InitPosition initPosition;

    EXPECT_TRUE(initPosition.loadMotorParameters());
    EXPECT_TRUE(initPosition.initializeSorterPosition());
    EXPECT_TRUE(initPosition.initializePositions());
}
