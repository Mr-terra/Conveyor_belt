// Log tests: the line format, the level labels, and truncation of long messages.

#include <gtest/gtest.h>

#include "log.h"

TEST(LogTest, LineHasLevelModuleAndMessage)
{
    testing::internal::CaptureStdout();

    LOG_WARN("Conveyor", "GRBL module at 0x%X", 0x70);

    EXPECT_EQ(testing::internal::GetCapturedStdout(), "[WARNING] Conveyor: GRBL module at 0x70\n");
}

TEST(LogTest, EachLevelHasItsOwnLabel)
{
    testing::internal::CaptureStdout();

    LOG_INFO("Scheduler", "started");
    LOG_WARN("Scheduler", "slow");
    LOG_ERROR("Scheduler", "failed");

    EXPECT_EQ(testing::internal::GetCapturedStdout(),
              "[INFO] Scheduler: started\n"
              "[WARNING] Scheduler: slow\n"
              "[ERROR] Scheduler: failed\n");
}

TEST(LogTest, LongMessagesAreTruncatedInsteadOfOverflowing)
{
    std::string longText(500, 'x');

    testing::internal::CaptureStdout();
    LOG_INFO("Test", "%s", longText.c_str());
    std::string out = testing::internal::GetCapturedStdout();

    EXPECT_LT(out.size(), 220u);
    EXPECT_EQ(out.substr(0, 17), "[INFO] Test: xxxx");
}
