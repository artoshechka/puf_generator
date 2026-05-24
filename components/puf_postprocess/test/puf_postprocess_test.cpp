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

// 1 раунд — результат совпадает с внутренним
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

// Побитовое большинство: последовательность {0xFF, 0xFF, 0x00} → 2 из 3 битов равны 1 → 0xFF
TEST(MajorityVoterTest, MajorityWins)
{
    std::vector<Fingerprint> seq = {{0xFF}, {0xFF}, {0x00}};
    auto voter = MajorityVoter(std::make_unique<MockPufGeneratorSequence>(seq), 3);
    Fingerprint result = voter.Generate();
    ASSERT_EQ(result.size(), 1u);
    EXPECT_EQ(result[0], 0xFF);
}

// FingerprintBits() делегирует внутреннему генератору
TEST(MajorityVoterTest, FingerprintBits)
{
    Fingerprint fp = {0x00, 0x00, 0x00};  // 3 байта = 24 бита
    auto voter = MajorityVoter(std::make_unique<MockPufGenerator>(fp), 3);
    EXPECT_EQ(voter.FingerprintBits(), 24u);
}

// ---------------------------------------------------------------------------
// VonNeumannDebias
// ---------------------------------------------------------------------------

// 0xAA в LSB-first порядке = биты 0,1,0,1,0,1,0,1 → пары (0,1),(0,1),(0,1),(0,1).
// Canonical Von Neumann: (0,1) → 0. targetBits=4 → 4 нулевых бита → младший полубайт 0x00.
TEST(VonNeumannDebiasTest, AlternatingBitsAA)
{
    auto debias = VonNeumannDebias(std::make_unique<MockPufGenerator>(Fingerprint{0xAA}), 4);
    Fingerprint result = debias.Generate();
    ASSERT_EQ(result.size(), 1u);
    EXPECT_EQ(result[0] & 0x0F, 0x00);
}

// 0x55 в LSB-first порядке = биты 1,0,1,0,1,0,1,0 → пары (1,0),(1,0),(1,0),(1,0).
// Canonical Von Neumann: (1,0) → 1. targetBits=4 → 4 единичных бита → младший полубайт 0x0F.
TEST(VonNeumannDebiasTest, AlternatingBits55)
{
    auto debias = VonNeumannDebias(std::make_unique<MockPufGenerator>(Fingerprint{0x55}), 4);
    Fingerprint result = debias.Generate();
    ASSERT_EQ(result.size(), 1u);
    EXPECT_EQ(result[0] & 0x0F, 0x0F);
}

// FingerprintBits() возвращает targetBits, а не длину внутреннего генератора
TEST(VonNeumannDebiasTest, FingerprintBits)
{
    auto debias = VonNeumannDebias(std::make_unique<MockPufGenerator>(Fingerprint{0xAA, 0x55}), 7);
    EXPECT_EQ(debias.FingerprintBits(), 7u);
}

// Первые 2 вызова внутреннего генератора возвращают 0x00 (все пары одинаковы → отбрасываем),
// третий возвращает 0xAA (LSB-first: пары (0,1) → canonical → биты 0). targetBits=4 → 0x00.
TEST(VonNeumannDebiasTest, AllSameBitsLoopsUntilUsable)
{
    std::vector<Fingerprint> seq = {{0x00}, {0x00}, {0xAA}};
    auto debias = VonNeumannDebias(std::make_unique<MockPufGeneratorSequence>(seq), 4);
    Fingerprint result = debias.Generate();
    ASSERT_EQ(result.size(), 1u);
    EXPECT_EQ(result[0] & 0x0F, 0x00);
}

// Источник всегда выдаёт одинаковые байты — у фон Неймана нет битовых пар (0,1)/(1,0),
// после maxAttempts попыток (targetBits * 100) должен бросить runtime_error.
TEST(VonNeumannDebiasTest, InsufficientEntropyThrows)
{
    auto debias = VonNeumannDebias(std::make_unique<MockPufGenerator>(Fingerprint{0x00}), 4);
    EXPECT_THROW((void)debias.Generate(), std::runtime_error);
}

// Граница большинства для 5 раундов: порог `votes > rounds/2 == 2`.
// 2 единицы из 5 не превышают порог → бит = 0.
TEST(MajorityVoterTest, FiveRoundsExactlyHalfStaysZero)
{
    std::vector<Fingerprint> seq = {{0xFF}, {0xFF}, {0x00}, {0x00}, {0x00}};
    auto voter = MajorityVoter(std::make_unique<MockPufGeneratorSequence>(seq), 5);
    Fingerprint result = voter.Generate();
    ASSERT_EQ(result.size(), 1u);
    EXPECT_EQ(result[0], 0x00u);
}

// 3 единицы из 5 превышают порог → бит = 1.
TEST(MajorityVoterTest, FiveRoundsJustOverHalfBecomesOne)
{
    std::vector<Fingerprint> seq = {{0xFF}, {0xFF}, {0xFF}, {0x00}, {0x00}};
    auto voter = MajorityVoter(std::make_unique<MockPufGeneratorSequence>(seq), 5);
    Fingerprint result = voter.Generate();
    ASSERT_EQ(result.size(), 1u);
    EXPECT_EQ(result[0], 0xFFu);
}
