/// @file puf_postprocess_test.cpp
/// @author Artemenko Anton
/// @brief Модульные тесты для VonNeumannDebias и MajorityVoter

#include <gtest/gtest.h>

#include <majority_voter.hpp>
#include <mock_puf_generator.hpp>
#include <von_neumann_debias.hpp>

using namespace puf;
using namespace puf::test;

// ---------------------------------------------------------------------------
// MajorityVoter
// ---------------------------------------------------------------------------

// 1 раунд — результат совпадает с inner
TEST(MajorityVoterTest, SingleRound)
{
    Fingerprint fp = {0xAB, 0xCD};
    auto voter = MajorityVoter(std::make_unique<MockPufGenerator>(fp), 1);
    EXPECT_EQ(voter.Generate(), fp);
}

// 3 одинаковых раунда → тот же вывод
TEST(MajorityVoterTest, AllSameRounds)
{
    Fingerprint fp = {0x5A};
    auto voter = MajorityVoter(std::make_unique<MockPufGenerator>(fp), 3);
    EXPECT_EQ(voter.Generate(), fp);
}

// Побитовое большинство: sequence {0xFF, 0xFF, 0x00} → 2 из 3 бит равны 1 → 0xFF
TEST(MajorityVoterTest, MajorityWins)
{
    std::vector<Fingerprint> seq = {{0xFF}, {0xFF}, {0x00}};
    auto voter = MajorityVoter(std::make_unique<MockPufGeneratorSequence>(seq), 3);
    Fingerprint result = voter.Generate();
    ASSERT_EQ(result.size(), 1u);
    EXPECT_EQ(result[0], 0xFF);
}

// FingerprintBits() делегирует в inner
TEST(MajorityVoterTest, FingerprintBits)
{
    Fingerprint fp = {0x00, 0x00, 0x00};  // 3 байта = 24 бита
    auto voter = MajorityVoter(std::make_unique<MockPufGenerator>(fp), 3);
    EXPECT_EQ(voter.FingerprintBits(), 24u);
}

// ---------------------------------------------------------------------------
// VonNeumannDebias
// ---------------------------------------------------------------------------

// 0xAA = 10101010: пары (1,0),(1,0),(1,0),(1,0) — каждая даёт бит 1
// targetBits=4: ожидаем 4 бита со значением 1 → младший ниббл = 0x0F
TEST(VonNeumannDebiasTest, AlternatingBitsAA)
{
    // 0xAA даёт 4 выходных бита=1 на 1 байт входа
    // targetBits=4 → 1 байт результата, биты 0..3 = 1 → 0x0F
    auto debias = VonNeumannDebias(std::make_unique<MockPufGenerator>(Fingerprint{0xAA}), 4);
    Fingerprint result = debias.Generate();
    ASSERT_EQ(result.size(), 1u);
    EXPECT_EQ(result[0] & 0x0F, 0x0F);
}

// 0x55 = 01010101: пары (0,1),(0,1),(0,1),(0,1) — каждая даёт бит 0
// targetBits=4 → результат 0x00
TEST(VonNeumannDebiasTest, AlternatingBits55)
{
    auto debias = VonNeumannDebias(std::make_unique<MockPufGenerator>(Fingerprint{0x55}), 4);
    Fingerprint result = debias.Generate();
    ASSERT_EQ(result.size(), 1u);
    EXPECT_EQ(result[0] & 0x0F, 0x00);
}

// FingerprintBits() возвращает targetBits, а не длину inner
TEST(VonNeumannDebiasTest, FingerprintBits)
{
    auto debias = VonNeumannDebias(std::make_unique<MockPufGenerator>(Fingerprint{0xAA, 0x55}), 7);
    EXPECT_EQ(debias.FingerprintBits(), 7u);
}

// Первые 2 вызова inner дают 0x00 (все пары одинаковы → отброс),
// третий даёт 0xAA (4 бита=1). targetBits=4 → результат не пустой.
TEST(VonNeumannDebiasTest, AllSameBitsLoopsUntilUsable)
{
    std::vector<Fingerprint> seq = {{0x00}, {0x00}, {0xAA}};
    auto debias = VonNeumannDebias(std::make_unique<MockPufGeneratorSequence>(seq), 4);
    Fingerprint result = debias.Generate();
    ASSERT_EQ(result.size(), 1u);
    // После двух пустых вызовов третий даёт 4 бита=1
    EXPECT_EQ(result[0] & 0x0F, 0x0F);
}
