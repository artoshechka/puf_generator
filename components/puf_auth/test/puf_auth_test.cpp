/// @file puf_auth_test.cpp
/// @author Artemenko Anton
/// @brief Unit tests for HammingAuthenticator

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
    // HD = 0 <= threshold → authentic
    EXPECT_TRUE(auth.Authenticate(ref));
}

TEST(HammingAuthenticator, AuthenticateCompletelyDifferent)
{
    Fingerprint ref = {0xFF};
    Fingerprint cand = {0x00};
    HammingAuthenticator auth(ref, 10.0);
    // HD = 100% > 10% → reject
    EXPECT_FALSE(auth.Authenticate(cand));
}

TEST(HammingAuthenticator, AuthenticateWithinThreshold)
{
    Fingerprint ref = {0xFF};         // 11111111
    Fingerprint cand = {0b11111110};  // 1 bit out of 8 → 12.5%
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

#endif  // GUID_C2E85A4F_71D3_4B8E_9F02_A1348DC6E507
