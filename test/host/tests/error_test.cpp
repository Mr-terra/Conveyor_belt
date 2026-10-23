#include <gtest/gtest.h>

#include "error.h"

class ErrorTest : public ::testing::Test
{
protected:
    void SetUp() override
    {
        hostMillis = 42;
        Error::clearCriticalErrors();
        Error::setErrorCallback(nullptr);
        Error::setCriticalErrorCallback(nullptr);
    }

    void TearDown() override
    {
        Error::clearCriticalErrors();
        Error::setErrorCallback(nullptr);
        Error::setCriticalErrorCallback(nullptr);
    }
};

TEST_F(ErrorTest, TriggerErrorForwardsMessageLevelAndModuleToCallback)
{
    String seenMessage, seenModule;
    ErrorLevel seenLevel = ERROR_FATAL;
    Error::setErrorCallback([&](const Error &error) {
        seenMessage = error.getMessage();
        seenModule = error.getModule();
        seenLevel = error.getLevel();
    });

    Error::triggerError("motor stalled", ERROR_WARNING, "Conveyor");

    EXPECT_EQ(seenMessage, "motor stalled");
    EXPECT_EQ(seenModule, "Conveyor");
    EXPECT_EQ(seenLevel, ERROR_WARNING);
}

TEST_F(ErrorTest, NonCriticalErrorDoesNotSetCriticalFlag)
{
    Error::triggerError("info", ERROR_INFO, "Test");
    Error::triggerError("warning", ERROR_WARNING, "Test");

    EXPECT_FALSE(Error::hasCriticalErrors());
}

TEST_F(ErrorTest, CriticalErrorSetsFlagAndCallsCriticalCallback)
{
    int criticalCalls = 0;
    Error::setCriticalErrorCallback([&]() { criticalCalls++; });

    Error::triggerError("overheat", ERROR_CRITICAL, "Conveyor");

    EXPECT_TRUE(Error::hasCriticalErrors());
    EXPECT_EQ(criticalCalls, 1);
}

TEST_F(ErrorTest, FatalErrorSetsFlagAndCallsCriticalCallback)
{
    int criticalCalls = 0;
    Error::setCriticalErrorCallback([&]() { criticalCalls++; });

    Error::triggerError("GRBL alarm", ERROR_FATAL, "Conveyor");

    EXPECT_TRUE(Error::hasCriticalErrors());
    EXPECT_EQ(criticalCalls, 1);
}

TEST_F(ErrorTest, ClearCriticalErrorsResetsFlag)
{
    Error::triggerError("overheat", ERROR_CRITICAL, "Conveyor");
    Error::clearCriticalErrors();

    EXPECT_FALSE(Error::hasCriticalErrors());
}

TEST_F(ErrorTest, ToStringContainsTimestampLevelModuleAndMessage)
{
    Error error("door open", ERROR_CRITICAL, "Sorter");

    EXPECT_EQ(error.toString(), String("[42] CRITICAL [Sorter]: door open"));
}

TEST_F(ErrorTest, ToStringUsesLevelName)
{
    EXPECT_EQ(Error("a", ERROR_INFO, "M").toString(), String("[42] INFO [M]: a"));
    EXPECT_EQ(Error("a", ERROR_WARNING, "M").toString(), String("[42] WARNING [M]: a"));
    EXPECT_EQ(Error("a", ERROR_FATAL, "M").toString(), String("[42] FATAL [M]: a"));
}
