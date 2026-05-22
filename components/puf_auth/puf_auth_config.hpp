/// @file puf_auth_config.hpp
/// @author Artemenko Anton
/// @brief Tunable defaults for the puf_auth module

#ifndef GUID_B04D9E82_1F3C_4C7A_A561_83E5D2C17F40
#define GUID_B04D9E82_1F3C_4C7A_A561_83E5D2C17F40

namespace puf
{

/// @brief Default intra-HD acceptance threshold for HammingAuthenticator, in percent.
///
/// A candidate fingerprint is accepted when its fractional Hamming distance
/// to the reference does not exceed this value / 100.
/// 10 % is conservative for a 256-bit RO PUF with no post-processing.
/// With MajorityVoter(3) the typical intra-HD drops below 2 %; 5 % is then safer.
inline constexpr double kDefaultThresholdPct = 10.0;

}  // namespace puf

#endif  // GUID_B04D9E82_1F3C_4C7A_A561_83E5D2C17F40
