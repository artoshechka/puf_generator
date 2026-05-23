/// @file sram_puf_test.cpp
/// @author Artemenko Anton
/// @brief Модульные тесты для SramPuf

#include <gtest/gtest.h>

#include <sram_puf.hpp>

using namespace puf;

static const uint8_t kData[] = {0xA5, 0x3C, 0xF0, 0x11, 0x22, 0x33, 0x44, 0x55};

TEST(SramPufTest, FingerprintBitsMatchesRequested)
{
    SramPuf puf(kData, sizeof(kData), 32);
    EXPECT_EQ(puf.FingerprintBits(), 32u);
}

TEST(SramPufTest, FingerprintVectorSizeIsCorrect)
{
    SramPuf puf(kData, sizeof(kData), 32);
    EXPECT_EQ(puf.Generate().size(), 4u);
}

TEST(SramPufTest, FingerprintMatchesSramData)
{
    SramPuf puf(kData, sizeof(kData), 24);
    Fingerprint fp = puf.Generate();
    ASSERT_EQ(fp.size(), 3u);
    EXPECT_EQ(fp[0], 0xA5u);
    EXPECT_EQ(fp[1], 0x3Cu);
    EXPECT_EQ(fp[2], 0xF0u);
}

TEST(SramPufTest, PartialByteAlignmentIsCorrect)
{
    SramPuf puf(kData, sizeof(kData), 9);
    EXPECT_EQ(puf.Generate().size(), 2u);
}

TEST(SramPufTest, SameBufferProducesSameFingerprint)
{
    SramPuf puf1(kData, sizeof(kData), 32);
    SramPuf puf2(kData, sizeof(kData), 32);
    EXPECT_EQ(puf1.Generate(), puf2.Generate());
}
