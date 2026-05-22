/// @file puf_auth_config.hpp
/// @author Artemenko Anton
/// @brief Настраиваемые значения по умолчанию для модуля puf_auth

#ifndef GUID_B04D9E82_1F3C_4C7A_A561_83E5D2C17F40
#define GUID_B04D9E82_1F3C_4C7A_A561_83E5D2C17F40

namespace puf
{

/// @brief Порог принятия кандидата по умолчанию для HammingAuthenticator, в процентах.
///
/// Кандидат считается подлинным, если дробное расстояние Хэмминга до эталона
/// не превышает это значение / 100.
/// 10 % — консервативный порог для 256-битного RO PUF без постобработки.
/// С MajorityVoter(3) типичный intra-HD опускается ниже 2 %; достаточно 5 %.
inline constexpr double kDefaultThresholdPct = 10.0;

}  // namespace puf

#endif  // GUID_B04D9E82_1F3C_4C7A_A561_83E5D2C17F40
