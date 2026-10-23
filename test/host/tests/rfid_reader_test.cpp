// RFIDReader tests: UID format, halting the card after each read,
// and the 1.5 s cooldown between reads of the same card.

#include <gmock/gmock.h>
#include <gtest/gtest.h>

#include <vector>

#include "hardware_mocks.h"
#include "reader.h"

using ::testing::_;
using ::testing::NiceMock;
using ::testing::Return;

class RfidReaderTest : public ::testing::Test
{
protected:
    void SetUp() override
    {
        hostMillis = 0;
        reader.setHardware(&driver, &bus);
    }

    // Simulates a card whose UID is the given bytes.
    void presentCard(std::initializer_list<uint8_t> bytes)
    {
        std::vector<uint8_t> uid(bytes); // owned copy, the action outlives this call
        ON_CALL(driver, isNewCardPresent()).WillByDefault(Return(true));
        ON_CALL(driver, readCardSerial()).WillByDefault(Return(true));
        ON_CALL(driver, uidBytes(_, _))
            .WillByDefault([uid](uint8_t *buffer, uint8_t maxLength) {
                uint8_t size = 0;
                for (uint8_t byte : uid)
                {
                    if (size < maxLength)
                        buffer[size++] = byte;
                }
                return size;
            });
    }

    NiceMock<MockRfidDriver> driver;
    NiceMock<MockI2CBus> bus;
    RFIDReader reader;
};

TEST_F(RfidReaderTest, NoCardReturnsEmptyUid)
{
    ON_CALL(driver, isNewCardPresent()).WillByDefault(Return(false));

    EXPECT_EQ(reader.readUid(), String(""));
}

TEST_F(RfidReaderTest, UnreadableCardReturnsEmptyUid)
{
    ON_CALL(driver, isNewCardPresent()).WillByDefault(Return(true));
    ON_CALL(driver, readCardSerial()).WillByDefault(Return(false));

    EXPECT_EQ(reader.readUid(), String(""));
}

TEST_F(RfidReaderTest, UidIsFormattedAsUppercaseHexPairs)
{
    presentCard({0x04, 0x1a, 0x2b, 0x0f});

    EXPECT_EQ(reader.readUid(), String("04:1A:2B:0F"));
}

TEST_F(RfidReaderTest, CardIsHaltedAfterEveryRead)
{
    presentCard({0x01, 0x02});

    EXPECT_CALL(driver, halt()).Times(1);
    reader.readUid();
}

TEST_F(RfidReaderTest, SecondReadWithinCooldownIsIgnored)
{
    presentCard({0xAA, 0xBB});

    hostMillis = 5000;
    EXPECT_EQ(reader.readUid(), String("AA:BB"));

    hostMillis = 5500;
    EXPECT_EQ(reader.readUid(), String(""));

    hostMillis = 7000;
    EXPECT_EQ(reader.readUid(), String("AA:BB"));
}

TEST_F(RfidReaderTest, RebindingHardwareResetsCooldown)
{
    presentCard({0xAA, 0xBB});
    hostMillis = 5000;
    EXPECT_EQ(reader.readUid(), String("AA:BB"));

    reader.setHardware(&driver, &bus);

    hostMillis = 5100;
    EXPECT_EQ(reader.readUid(), String("AA:BB"));
}

TEST_F(RfidReaderTest, CheckConnectionUsesRfidI2cAddress)
{
    EXPECT_CALL(bus, probe(0x28)).WillOnce(Return(0)).WillOnce(Return(2));
    EXPECT_TRUE(reader.checkConnection());
    EXPECT_FALSE(reader.checkConnection());
}
