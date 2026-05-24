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

TEST(SramPufTest, NullptrBaseThrows)
{
    EXPECT_THROW(SramPuf(nullptr, 8, 32), std::invalid_argument);
}

TEST(SramPufTest, BitsExceedBufferThrows)
{
    EXPECT_THROW(SramPuf(kData, sizeof(kData), sizeof(kData) * 8U + 1U),
                 std::invalid_argument);
}

// Регрессия на volatile: если убрать volatile из SramPuf::base_, компилятор
// (в Release с LTO) может закэшировать первое чтение и пропустить повторные.
// Меняем содержимое источника между вызовами Generate() и убеждаемся, что
// результат отражает изменение.
TEST(SramPufTest, VolatileObservesMutation)
{
    static volatile uint8_t mutable_src[4] = {0x11, 0x22, 0x33, 0x44};
    SramPuf puf(const_cast<const volatile uint8_t*>(mutable_src), sizeof(mutable_src), 32);

    Fingerprint first = puf.Generate();
    ASSERT_EQ(first.size(), 4u);
    EXPECT_EQ(first[0], 0x11u);

    mutable_src[0] = 0xAB;
    Fingerprint second = puf.Generate();
    ASSERT_EQ(second.size(), 4u);
    EXPECT_EQ(second[0], 0xABu);
}
