/// @file hamming_authenticator.hpp
/// @author Artemenko Anton
/// @brief Authenticator based on Hamming distance

#ifndef GUID_F712E89C_625A_4C4E_AC3B_29B337659DF0
#define GUID_F712E89C_625A_4C4E_AC3B_29B337659DF0

#include <i_authenticator.hpp>
#include <puf_auth_config.hpp>

namespace puf
{

/// @brief Compares fingerprints by Hamming distance.
/// Accepts a candidate as authentic if the fraction of differing bits
/// does not exceed the threshold (default 10%).
class HammingAuthenticator final : public IAuthenticator
{
   public:
    /// @param[in] reference    Reference fingerprint (enrolled)
    /// @param[in] thresholdPct Maximum allowable intra-HD in percent [0..100]
    HammingAuthenticator(Fingerprint reference, double thresholdPct = kDefaultThresholdPct);

    /// @brief Compares candidate against the reference by Hamming distance
    /// @param[in] candidate Fingerprint presented during authentication
    /// @return true if the fractional HD does not exceed thresholdPct_/100
    bool Authenticate(const Fingerprint& candidate) override;

    /// @brief Counts the number of differing bits between two fingerprints
    /// @return Hamming distance in bits
    static size_t HammingDistance(const Fingerprint& a, const Fingerprint& b);

    /// @brief Normalizes the Hamming distance by the fingerprint length
    /// @return Fractional Hamming distance [0.0, 1.0]
    static double FractionalHD(const Fingerprint& a, const Fingerprint& b);

   private:
    Fingerprint reference_;  ///< Reference fingerprint registered during enrollment
    double thresholdPct_;    ///< Acceptance threshold in percent [0..100]
};

}  // namespace puf

#endif  // GUID_F712E89C_625A_4C4E_AC3B_29B337659DF0
