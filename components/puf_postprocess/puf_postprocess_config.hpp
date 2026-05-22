/// @file puf_postprocess_config.hpp
/// @author Artemenko Anton
/// @brief Configurable default values for puf_postprocess decorators

#ifndef GUID_E7B20F41_934C_4AE1_8D67_C5102DF93BA1
#define GUID_E7B20F41_934C_4AE1_8D67_C5102DF93BA1

#include <cstddef>

namespace puf
{

/// @brief Default number of majority voting rounds for MajorityVoter.
///
/// Must be odd for unambiguous majority. 3 rounds is the optimal balance
/// of stability and speed for most RO PUF configurations.
inline constexpr size_t kDefaultMajorityRounds = 3;

}  // namespace puf

#endif  // GUID_E7B20F41_934C_4AE1_8D67_C5102DF93BA1
