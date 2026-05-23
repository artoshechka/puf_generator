/// @file puf_metrics_test.cpp
/// @author Artemenko Anton
/// @brief Модульные тесты для puf::metrics

#ifndef GUID_3A7F1C9E_B842_4D5F_A610_E98C2D047F31
#define GUID_3A7F1C9E_B842_4D5F_A610_E98C2D047F31

#include <gtest/gtest.h>

#include <puf_metrics.hpp>

using puf::Fingerprint;
using namespace puf::metrics;

// --- HammingDistance ---

TEST(HammingDistance, HammingDistanceIdentical)
{
    Fingerprint a = {0xAB, 0xCD};
    Fingerprint b = {0xAB, 0xCD};
    EXPECT_EQ(HammingDistance(a, b), 0u);
}

TEST(HammingDistance, HammingDistanceAllFlipped)
{
    Fingerprint a = {0x00};
    Fingerprint b = {0xFF};
    EXPECT_EQ(HammingDistance(a, b), 8u);
}

TEST(HammingDistance, HammingDistanceSingleBit)
{
    Fingerprint a = {0x01};
    Fingerprint b = {0x00};
    EXPECT_EQ(HammingDistance(a, b), 1u);
}

// --- FractionalHD ---

TEST(FractionalHD, FractionalHDZero)
{
    Fingerprint a = {0xAB, 0xCD};
    Fingerprint b = {0xAB, 0xCD};
    EXPECT_DOUBLE_EQ(FractionalHD(a, b), 0.0);
}

TEST(FractionalHD, FractionalHDMax)
{
    Fingerprint a = {0x00};
    Fingerprint b = {0xFF};
    EXPECT_DOUBLE_EQ(FractionalHD(a, b), 1.0);
}

TEST(FractionalHD, FractionalHDHalf)
{
    // 0x0F = 00001111, 0xF0 = 11110000 — все 8 битов различаются
    Fingerprint a = {0x0F};
    Fingerprint b = {0xF0};
    EXPECT_DOUBLE_EQ(FractionalHD(a, b), 1.0);
}

// --- IntraHD ---

TEST(IntraHD, IntraHDThrowsOnSingleSample)
{
    std::vector<Fingerprint> samples = {{0xAA}};
    EXPECT_THROW(IntraHD(samples), std::invalid_argument);
}

TEST(IntraHD, IntraHDIdenticalSamples)
{
    std::vector<Fingerprint> samples = {{0xAA}, {0xAA}};
    EXPECT_DOUBLE_EQ(IntraHD(samples), 0.0);
}

TEST(IntraHD, IntraHDTwoSamples)
{
    // 0xFF и 0x00 — 8 из 8 битов различаются → HD = 1.0
    std::vector<Fingerprint> samples = {{0xFF}, {0x00}};
    EXPECT_DOUBLE_EQ(IntraHD(samples), 1.0);
}

// --- InterHD ---

TEST(InterHD, InterHDThrowsOnSingleDevice)
{
    std::vector<Fingerprint> devices = {{0xAA}};
    EXPECT_THROW(InterHD(devices), std::invalid_argument);
}

TEST(InterHD, InterHDMaxDistinct)
{
    // {0xFF} против {0x00} → HD = 1.0
    std::vector<Fingerprint> devices = {{0xFF}, {0x00}};
    EXPECT_DOUBLE_EQ(InterHD(devices), 1.0);
}

TEST(InterHD, InterHDIdentical)
{
    std::vector<Fingerprint> devices = {{0xAA}, {0xAA}};
    EXPECT_DOUBLE_EQ(InterHD(devices), 0.0);
}

// --- Uniformity ---

TEST(Uniformity, UniformityAllZero)
{
    Fingerprint fp = {0x00};
    EXPECT_DOUBLE_EQ(Uniformity(fp), 0.0);
}

TEST(Uniformity, UniformityAllOne)
{
    Fingerprint fp = {0xFF};
    EXPECT_DOUBLE_EQ(Uniformity(fp), 1.0);
}

TEST(Uniformity, UniformityHalf)
{
    // 0xAA = 10101010 → 4 единицы из 8
    Fingerprint fp = {0xAA};
    EXPECT_DOUBLE_EQ(Uniformity(fp), 0.5);
}

#endif  // GUID_3A7F1C9E_B842_4D5F_A610_E98C2D047F31
