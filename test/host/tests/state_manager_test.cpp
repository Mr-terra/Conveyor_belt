#include <gtest/gtest.h>

#include <vector>

#include "error.h"
#include "scheduler.h"
#include "state_manager.h"

namespace
{
    String identity(String s) { return s; }
}

class StateManagerTest : public ::testing::Test
{
protected:
    void SetUp() override
    {
        hostMillis = 0;
        Error::clearCriticalErrors();
        Error::setErrorCallback([this](const Error &error) { errors.push_back(error); });
        Error::setCriticalErrorCallback(nullptr);
    }

    void TearDown() override
    {
        Error::clearCriticalErrors();
        Error::setErrorCallback(nullptr);
    }

    bool sawMessage(const String &message) const
    {
        for (const Error &error : errors)
        {
            if (error.getMessage() == message)
                return true;
        }
        return false;
    }

    int countWarnings() const
    {
        int count = 0;
        for (const Error &error : errors)
        {
            if (error.getLevel() == ERROR_WARNING)
                count++;
        }
        return count;
    }

    StateManager manager;
    std::vector<Error> errors;
};

TEST_F(StateManagerTest, RuleFiresOnMatchingTransition)
{
    String state = "IDLE";
    manager.observeState<String>(&state, "Module", identity);
    manager.addStateRule("Module", "IDLE", "BUSY", ERROR_INFO, "became busy");

    state = "BUSY";
    manager.monitorAllStates();

    EXPECT_TRUE(sawMessage("became busy"));
}

TEST_F(StateManagerTest, WildcardFromStateMatchesAnyPreviousState)
{
    String state = "IDLE";
    manager.observeState<String>(&state, "Module", identity);
    manager.addStateRule("Module", "*", "ERROR", ERROR_CRITICAL, "any to error");

    state = "RUNNING";
    manager.monitorAllStates();
    state = "ERROR";
    manager.monitorAllStates();

    EXPECT_TRUE(sawMessage("any to error"));
}

TEST_F(StateManagerTest, RuleIgnoresOtherModules)
{
    String state = "IDLE";
    manager.observeState<String>(&state, "Module", identity);
    manager.addStateRule("Other", "IDLE", "BUSY", ERROR_INFO, "wrong module");

    state = "BUSY";
    manager.monitorAllStates();

    EXPECT_FALSE(sawMessage("wrong module"));
}

TEST_F(StateManagerTest, NoRuleEvaluationWithoutStateChange)
{
    String state = "IDLE";
    manager.observeState<String>(&state, "Module", identity);
    manager.addStateRule("Module", "IDLE", "IDLE", ERROR_INFO, "should not fire");

    manager.monitorAllStates();
    manager.monitorAllStates();

    EXPECT_FALSE(sawMessage("should not fire"));
}

TEST_F(StateManagerTest, TransitionalStateWarnsOnceAfterTimeout)
{
    String state = "CHECK_CONNECT";
    manager.observeState<String>(&state, "Scheduler", identity);

    hostMillis = 4000;
    manager.monitorAllStates();
    EXPECT_EQ(countWarnings(), 0) << "no warning before the 5s timeout";

    hostMillis = 6000;
    manager.monitorAllStates();
    manager.monitorAllStates();

    EXPECT_EQ(countWarnings(), 1);
    EXPECT_TRUE(sawMessage("Scheduler stuck in state: CHECK_CONNECT"));
}

TEST_F(StateManagerTest, SteadyOperatingStatesAreNotReportedAsStuck)
{
    String state = "RUNNING";
    manager.observeState<String>(&state, "Conveyor", identity);

    hostMillis = 60000;
    manager.monitorAllStates();

    EXPECT_EQ(countWarnings(), 0) << "a conveyor running normally is not stuck";
}

TEST_F(StateManagerTest, IdleStatesAreNeverReportedAsStuck)
{
    String state = "WAITING";
    manager.observeState<String>(&state, "Scheduler", identity);

    hostMillis = 60000;
    manager.monitorAllStates();

    EXPECT_EQ(countWarnings(), 0);
}

TEST_F(StateManagerTest, ConveyorAlarmDuringProductionReturnsSchedulerToWaiting)
{
    Scheduler scheduler;
    scheduler.initialize();
    scheduler.setState(STATE_PRODUCTION);
    manager.setScheduler(&scheduler);

    String conveyor = "RUNNING";
    manager.observeState<String>(&conveyor, "Conveyor", identity);
    manager.addStateRule("Conveyor", "*", "ALARM", ERROR_CRITICAL, "alarm");

    conveyor = "ALARM";
    manager.monitorAllStates();

    EXPECT_EQ(scheduler.getState(), STATE_WAITING);
}

TEST_F(StateManagerTest, FatalConveyorAlarmPutsSchedulerInError)
{
    Scheduler scheduler;
    scheduler.initialize();
    scheduler.setState(STATE_PRODUCTION);
    manager.setScheduler(&scheduler);

    String conveyor = "RUNNING";
    manager.observeState<String>(&conveyor, "Conveyor", identity);
    manager.addStateRule("Conveyor", "*", "ALARM", ERROR_FATAL, "fatal alarm");

    conveyor = "ALARM";
    manager.monitorAllStates();

    EXPECT_EQ(scheduler.getState(), STATE_ERROR);
}

TEST_F(StateManagerTest, AlarmOutsideProductionKeepsSchedulerState)
{
    Scheduler scheduler;
    scheduler.initialize();
    scheduler.setState(STATE_CHECK_CONNECT);
    manager.setScheduler(&scheduler);

    String conveyor = "RUNNING";
    manager.observeState<String>(&conveyor, "Conveyor", identity);
    manager.addStateRule("Conveyor", "*", "ALARM", ERROR_CRITICAL, "alarm");

    conveyor = "ALARM";
    manager.monitorAllStates();

    EXPECT_EQ(scheduler.getState(), STATE_CHECK_CONNECT);
}
