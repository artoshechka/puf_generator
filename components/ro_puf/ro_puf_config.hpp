/// @file ro_puf_config.hpp
/// @author Artemenko Anton
/// @brief Настраиваемые значения по умолчанию для генератора RoPuf

#ifndef GUID_A1C3F027_8E54_4D1B_B902_FC2D7E4A5019
#define GUID_A1C3F027_8E54_4D1B_B902_FC2D7E4A5019

#include <cstdint>

namespace puf
{

/// @brief Окно измерения осциллятора по умолчанию в тактах CPU.
///
/// Большие значения повышают стабильность (меньший intra-HD), но увеличивают
/// время генерации. 200 000 тактов ≈ 100 мкс при частоте 240 МГц.
inline constexpr uint32_t kDefaultWindowCycles = 200'000;

}  // namespace puf

#endif  // GUID_A1C3F027_8E54_4D1B_B902_FC2D7E4A5019
