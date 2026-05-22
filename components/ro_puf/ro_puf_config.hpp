/// @file ro_puf_config.hpp
/// @author Artemenko Anton
/// @brief Tunable defaults for the RoPuf generator

#ifndef GUID_A1C3F027_8E54_4D1B_B902_FC2D7E4A5019
#define GUID_A1C3F027_8E54_4D1B_B902_FC2D7E4A5019

#include <cstdint>

namespace puf
{

/// @brief Default oscillator measurement window in CPU cycles.
///
/// Larger values increase stability (lower intra-HD) at the cost of
/// longer generation time. 200 000 cycles ≈ 100 µs at 240 MHz.
inline constexpr uint32_t kDefaultWindowCycles = 200'000;

}  // namespace puf

#endif  // GUID_A1C3F027_8E54_4D1B_B902_FC2D7E4A5019
