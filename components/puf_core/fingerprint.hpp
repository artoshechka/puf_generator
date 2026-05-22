/// @file fingerprint.hpp
/// @author Artemenko Anton
/// @brief Device fingerprint type

#ifndef GUID_0448F913_446D_4ECC_B55C_D1CED83A748D
#define GUID_0448F913_446D_4ECC_B55C_D1CED83A748D

#include <cstdint>
#include <vector>

namespace puf
{

/// @brief Binary device fingerprint — sequence of bytes
using Fingerprint = std::vector<uint8_t>;

}  // namespace puf

#endif  // GUID_0448F913_446D_4ECC_B55C_D1CED83A748D
