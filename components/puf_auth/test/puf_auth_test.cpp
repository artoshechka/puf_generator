/// @file puf_auth_test.cpp
/// @author Artemenko Anton
/// @brief Модульные тесты для HammingAuthenticator

#ifndef GUID_C2E85A4F_71D3_4B8E_9F02_A1348DC6E507
#define GUID_C2E85A4F_71D3_4B8E_9F02_A1348DC6E507

#include <gtest/gtest.h>

#include <hamming_authenticator.hpp>

using puf::Fingerprint;
using puf::HammingAuthenticator;

TEST(HammingAuthenticator, AuthenticateSameFingerprint)
{
    Fingerprint ref = {0xAB, 0xCD};
    HammingAuthenticator auth(ref, 10.0);
    // HD = 0 <= порог → подлинный
    EXPECT_TRUE(auth.Authenticate(ref));
}

TEST(HammingAuthenticator, AuthenticateCompletelyDifferent)
{
    Fingerprint ref = {0xFF};
    Fingerprint cand = {0x00};
    HammingAuthenticator auth(ref, 10.0);
    // HD = 100% > 10% → отклонить
    EXPECT_FALSE(auth.Authenticate(cand));
}

TEST(HammingAuthenticator, AuthenticateWithinThreshold)
{
    Fingerprint ref = {0xFF};         // 11111111
    Fingerprint cand = {0b11111110};  // 1 бит из 8 → 12.5%
    HammingAuthenticator auth15(ref, 15.0);
    EXPECT_TRUE(auth15.Authenticate(cand));  // 12.5 <= 15.0 → true
    HammingAuthenticator auth10(ref, 10.0);
    EXPECT_FALSE(auth10.Authenticate(cand));  // 12.5 > 10.0 → false
}

TEST(HammingAuthenticator, InvalidThresholdThrows)
{
    Fingerprint ref = {0xAB};
    EXPECT_THROW(HammingAuthenticator(ref, -1.0), std::invalid_argument);
}

TEST(HammingAuthenticator, InvalidThresholdAbove100Throws)
{
    Fingerprint ref = {0xAB};
    EXPECT_THROW(HammingAuthenticator(ref, 101.0), std::invalid_argument);
}

TEST(HammingAuthenticator, HammingDistanceStatic)
{
    EXPECT_EQ(HammingAuthenticator::HammingDistance({0xFF}, {0x00}), 8u);
}

TEST(HammingAuthenticator, FractionalHDStatic)
{
    EXPECT_DOUBLE_EQ(HammingAuthenticator::FractionalHD({0xFF}, {0x00}), 1.0);
}

TEST(HammingAuthenticator, HammingDistanceMismatchThrows)
{
    EXPECT_THROW(HammingAuthenticator::HammingDistance({0xAA}, {0xAA, 0xBB}),
                 std::invalid_argument);
}

TEST(HammingAuthenticator, FractionalHDMismatchThrows)
{
    EXPECT_THROW(HammingAuthenticator::FractionalHD({0xAA}, {0xAA, 0xBB}),
                 std::invalid_argument);
}

TEST(HammingAuthenticator, FractionalHDEmptyThrows)
{
    EXPECT_THROW(HammingAuthenticator::FractionalHD({}, {0xAA}),
                 std::invalid_argument);
}

TEST(HammingAuthenticator, AuthenticateRejectsSizeMismatch)
{
    Fingerprint ref = {0xAB, 0xCD};
    HammingAuthenticator auth(ref, 10.0);
    Fingerprint shorter = {0xAB};
    EXPECT_FALSE(auth.Authenticate(shorter));
}

TEST(HammingAuthenticator, AuthenticateRejectsLongerCandidate)
{
    Fingerprint ref = {0xAB, 0xCD};
    HammingAuthenticator auth(ref, 50.0);
    Fingerprint longer = {0xAB, 0xCD, 0x00};
    // Длина не совпадает → mismatchMask поднимает дистанцию до refBits → > threshold.
    EXPECT_FALSE(auth.Authenticate(longer));
}

TEST(HammingAuthenticator, AuthenticateThresholdBoundaryInteger)
{
    // 1 байт = 8 бит; threshold 12.5% → 1 бит.
    Fingerprint ref = {0xFF};
    HammingAuthenticator auth(ref, 12.5);
    Fingerprint oneFlip = {0b11111110};
    EXPECT_TRUE(auth.Authenticate(oneFlip));  // dist=1, threshold=1
    Fingerprint twoFlips = {0b11111100};
    EXPECT_FALSE(auth.Authenticate(twoFlips));  // dist=2, threshold=1
}

TEST(HammingAuthenticator, AuthenticateZeroThresholdRequiresExactMatch)
{
    Fingerprint ref = {0xAB, 0xCD};
    HammingAuthenticator auth(ref, 0.0);
    EXPECT_TRUE(auth.Authenticate(ref));
    Fingerprint oneBitOff = {0xAA, 0xCD};
    EXPECT_FALSE(auth.Authenticate(oneBitOff));
}

#endif  // GUID_C2E85A4F_71D3_4B8E_9F02_A1348DC6E507
