/// @file puf_postprocess_config.hpp
/// @author Artemenko Anton
/// @brief Настраиваемые значения по умолчанию для декораторов puf_postprocess

#ifndef GUID_E7B20F41_934C_4AE1_8D67_C5102DF93BA1
#define GUID_E7B20F41_934C_4AE1_8D67_C5102DF93BA1

#include <cstddef>

namespace puf
{

/// @brief Количество раундов мажоритарного голосования по умолчанию для MajorityVoter.
///
/// Должно быть нечётным для однозначного большинства. 3 раунда — оптимальный баланс
/// стабильности и скорости для типовых сценариев SRAM PUF.
inline constexpr size_t kDefaultMajorityRounds = 3;

}  // namespace puf

#endif  // GUID_E7B20F41_934C_4AE1_8D67_C5102DF93BA1
