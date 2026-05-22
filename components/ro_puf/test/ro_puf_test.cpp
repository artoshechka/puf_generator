/// @file ro_puf_test.cpp
/// @author Artemenko Anton
/// @brief Модульные тесты для RoPuf

#include <gtest/gtest.h>

#include <mock_oscillator.hpp>
#include <ro_puf.hpp>

using namespace puf;
using namespace puf::test;

static std::vector<std::unique_ptr<IOscillator>> MakeOscillators(std::initializer_list<uint32_t> counts)
{
    std::vector<std::unique_ptr<IOscillator>> oscs;
    for (uint32_t c : counts)
    {
        oscs.emplace_back(std::make_unique<MockOscillator>(c));
    }
    return oscs;
}

// Два осциллятора: counts[0] > counts[1] → бит 0 равен 1
TEST(RoPufTest, HigherCountProducesOneBit)
{
    auto puf = RoPuf(MakeOscillators({200, 100}), 1);
    Fingerprint fp = puf.Generate();
    EXPECT_EQ(fp[0] & 0x01, 1);
}

// Два осциллятора: counts[0] < counts[1] → бит 0 равен 0
TEST(RoPufTest, LowerCountProducesZeroBit)
{
    auto puf = RoPuf(MakeOscillators({100, 200}), 1);
    Fingerprint fp = puf.Generate();
    EXPECT_EQ(fp[0] & 0x01, 0);
}

// FingerprintBits() возвращает заданную длину
TEST(RoPufTest, FingerprintBitsMatchesRequested)
{
    auto puf = RoPuf(MakeOscillators({1, 2, 3, 4}), 6);
    EXPECT_EQ(puf.FingerprintBits(), 6u);
}

// Размер вектора соответствует ceil(bits/8)
TEST(RoPufTest, FingerprintVectorSizeIsCorrect)
{
    auto puf = RoPuf(MakeOscillators({1, 2, 3, 4, 5}), 10);
    EXPECT_EQ(puf.Generate().size(), 2u);
}

// Детерминированность: одинаковые счётчики → одинаковый отпечаток
TEST(RoPufTest, SameCountsProduceSameFingerprint)
{
    auto puf1 = RoPuf(MakeOscillators({300, 100, 200}), 3);
    auto puf2 = RoPuf(MakeOscillators({300, 100, 200}), 3);
    EXPECT_EQ(puf1.Generate(), puf2.Generate());
}

// Разные счётчики → разные отпечатки
TEST(RoPufTest, DifferentCountsProduceDifferentFingerprints)
{
    auto puf1 = RoPuf(MakeOscillators({300, 100, 200}), 3);
    auto puf2 = RoPuf(MakeOscillators({100, 300, 200}), 3);
    EXPECT_NE(puf1.Generate(), puf2.Generate());
}
