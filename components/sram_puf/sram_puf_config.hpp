/// @file sram_puf_config.hpp
/// @author Artemenko Anton
/// @brief Значения по умолчанию для генератора SramPuf

#ifndef GUID_C4B7A2E9_3D51_4F6C_8B0E_5A17D9F3C284
#define GUID_C4B7A2E9_3D51_4F6C_8B0E_5A17D9F3C284

#include <cstddef>

namespace puf
{

/// @brief Размер области SRAM PUF по умолчанию в байтах (512 бит).
inline constexpr size_t kDefaultSramPufBytes = 64U;

}  // namespace puf

#endif  // GUID_C4B7A2E9_3D51_4F6C_8B0E_5A17D9F3C284
