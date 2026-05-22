/// @file puf_postprocess_test.cpp
/// @author Artemenko Anton
/// @brief Unit tests for VonNeumannDebias and MajorityVoter

#include <gtest/gtest.h>

#include <majority_voter.hpp>
#include <mock_puf_generator.hpp>
#include <von_neumann_debias.hpp>

using namespace puf;
using namespace puf::test;

// ---------------------------------------------------------------------------
// MajorityVoter
// ---------------------------------------------------------------------------

// 1 round — result matches inner
TEST(MajorityVoterTest, SingleRound)
{
    Fingerprint fp = {0xAB, 0xCD};
    auto voter = MajorityVoter(std::make_unique<MockPufGenerator>(fp), 1);
    EXPECT_EQ(voter.Generate(), fp);
}

// 3 identical rounds → same output
TEST(MajorityVoterTest, AllSameRounds)
{
    Fingerprint fp = {0x5A};
    auto voter = MajorityVoter(std::make_unique<MockPufGenerator>(fp), 3);
    EXPECT_EQ(voter.Generate(), fp);
}

// Bitwise majority: sequence {0xFF, 0xFF, 0x00} → 2 out of 3 bits are 1 → 0xFF
TEST(MajorityVoterTest, MajorityWins)
{
    std::vector<Fingerprint> seq = {{0xFF}, {0xFF}, {0x00}};
    auto voter = MajorityVoter(std::make_unique<MockPufGeneratorSequence>(seq), 3);
    Fingerprint result = voter.Generate();
    ASSERT_EQ(result.size(), 1u);
    EXPECT_EQ(result[0], 0xFF);
}

// FingerprintBits() delegates to inner
TEST(MajorityVoterTest, FingerprintBits)
{
    Fingerprint fp = {0x00, 0x00, 0x00};  // 3 bytes = 24 bits
    auto voter = MajorityVoter(std::make_unique<MockPufGenerator>(fp), 3);
    EXPECT_EQ(voter.FingerprintBits(), 24u);
}

// ---------------------------------------------------------------------------
// VonNeumannDebias
// ---------------------------------------------------------------------------

// 0xAA = 10101010: pairs (1,0),(1,0),(1,0),(1,0) — each yields bit 1
// targetBits=4: expect 4 bits equal to 1 → low nibble = 0x0F
TEST(VonNeumannDebiasTest, AlternatingBitsAA)
{
    // 0xAA produces 4 output bits=1 per 1 input byte
    // targetBits=4 → 1 result byte, bits 0..3 = 1 → 0x0F
    auto debias = VonNeumannDebias(std::make_unique<MockPufGenerator>(Fingerprint{0xAA}), 4);
    Fingerprint result = debias.Generate();
    ASSERT_EQ(result.size(), 1u);
    EXPECT_EQ(result[0] & 0x0F, 0x0F);
}

// 0x55 = 01010101: pairs (0,1),(0,1),(0,1),(0,1) — each yields bit 0
// targetBits=4 → result 0x00
TEST(VonNeumannDebiasTest, AlternatingBits55)
{
    auto debias = VonNeumannDebias(std::make_unique<MockPufGenerator>(Fingerprint{0x55}), 4);
    Fingerprint result = debias.Generate();
    ASSERT_EQ(result.size(), 1u);
    EXPECT_EQ(result[0] & 0x0F, 0x00);
}

// FingerprintBits() returns targetBits, not the length of inner
TEST(VonNeumannDebiasTest, FingerprintBits)
{
    auto debias = VonNeumannDebias(std::make_unique<MockPufGenerator>(Fingerprint{0xAA, 0x55}), 7);
    EXPECT_EQ(debias.FingerprintBits(), 7u);
}

// First 2 calls to inner return 0x00 (all pairs identical → discard),
// third returns 0xAA (4 bits=1). targetBits=4 → result is non-empty.
TEST(VonNeumannDebiasTest, AllSameBitsLoopsUntilUsable)
{
    std::vector<Fingerprint> seq = {{0x00}, {0x00}, {0xAA}};
    auto debias = VonNeumannDebias(std::make_unique<MockPufGeneratorSequence>(seq), 4);
    Fingerprint result = debias.Generate();
    ASSERT_EQ(result.size(), 1u);
    // After two empty calls, the third yields 4 bits=1
    EXPECT_EQ(result[0] & 0x0F, 0x0F);
}
