/// @file puf_postprocess_config.hpp
/// @author Artemenko Anton
/// @brief Tunable defaults for puf_postprocess decorators

#ifndef GUID_E7B20F41_934C_4AE1_8D67_C5102DF93BA1
#define GUID_E7B20F41_934C_4AE1_8D67_C5102DF93BA1

#include <cstddef>

namespace puf
{

/// @brief Default number of majority-vote rounds for MajorityVoter.
///
/// Must be odd to guarantee a clear majority. 3 rounds gives a good
/// stability/speed trade-off for most RO PUF deployments.
inline constexpr size_t kDefaultMajorityRounds = 3;

}  // namespace puf

#endif  // GUID_E7B20F41_934C_4AE1_8D67_C5102DF93BA1
