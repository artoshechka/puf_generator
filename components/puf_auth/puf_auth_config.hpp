/// @file puf_auth_config.hpp
/// @author Artemenko Anton
/// @brief Configurable default values for the puf_auth module

#ifndef GUID_B04D9E82_1F3C_4C7A_A561_83E5D2C17F40
#define GUID_B04D9E82_1F3C_4C7A_A561_83E5D2C17F40

namespace puf
{

/// @brief Default acceptance threshold for HammingAuthenticator, in percent.
///
/// A candidate is considered authentic if the fractional Hamming distance to the reference
/// does not exceed this value / 100.
/// 10% is a conservative threshold for a 256-bit RO PUF without post-processing.
/// With MajorityVoter(3), typical intra-HD drops below 2%; 5% is sufficient.
inline constexpr double kDefaultThresholdPct = 10.0;

}  // namespace puf

#endif  // GUID_B04D9E82_1F3C_4C7A_A561_83E5D2C17F40
