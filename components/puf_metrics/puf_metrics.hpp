/// @file puf_metrics.hpp
/// @author Artemenko Anton
/// @brief PUF fingerprint quality evaluation metrics

#ifndef GUID_D68368E0_2ED9_49ED_BA95_B2099115DCDD
#define GUID_D68368E0_2ED9_49ED_BA95_B2099115DCDD

#include <fingerprint.hpp>
#include <vector>

namespace puf::metrics
{

/// @brief Counts the number of differing bits between two fingerprints
/// @param[in] a First fingerprint
/// @param[in] b Second fingerprint
/// @return Hamming distance in bits
size_t HammingDistance(const Fingerprint& a, const Fingerprint& b);

/// @brief Normalizes Hamming distance by fingerprint length
/// @param[in] a First fingerprint
/// @param[in] b Second fingerprint
/// @return Fractional Hamming distance [0.0, 1.0]
double FractionalHD(const Fingerprint& a, const Fingerprint& b);

/// @brief Intra-device HD: average HD between repeated measurements of one device.
/// Ideal value: 0.0 (full stability). Acceptable: < 0.05.
/// @param[in] samples Set of measurements from one device (minimum 2)
/// @return Average fractional HD across all measurement pairs
/// @throws std::invalid_argument if samples contains fewer than 2 elements
double IntraHD(const std::vector<Fingerprint>& samples);

/// @brief Inter-device HD: average HD between fingerprints of different devices.
/// Ideal value: 0.5 (maximum distinctiveness).
/// @param[in] deviceFingerprints One fingerprint per device (minimum 2)
/// @return Average fractional HD across all device pairs
/// @throws std::invalid_argument if deviceFingerprints contains fewer than 2 elements
double InterHD(const std::vector<Fingerprint>& deviceFingerprints);

/// @brief Fraction of bits equal to 1 — measure of distribution uniformity.
/// Ideal value: 0.5.
/// @param[in] fp Fingerprint to analyze
/// @return Fraction of set bits [0.0, 1.0]
double Uniformity(const Fingerprint& fp);

}  // namespace puf::metrics

#endif  // GUID_D68368E0_2ED9_49ED_BA95_B2099115DCDD
