/// @file fingerprint.hpp
/// @author Artemenko Anton
/// @brief Тип отпечатка устройства

#ifndef GUID_0448F913_446D_4ECC_B55C_D1CED83A748D
#define GUID_0448F913_446D_4ECC_B55C_D1CED83A748D

#include <cstdint>
#include <vector>

namespace puf
{

/// @brief Бинарный отпечаток устройства — последовательность байт
using Fingerprint = std::vector<uint8_t>;

}  // namespace puf

#endif  // GUID_0448F913_446D_4ECC_B55C_D1CED83A748D
