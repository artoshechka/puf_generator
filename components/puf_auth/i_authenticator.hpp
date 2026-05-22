/// @file i_authenticator.hpp
/// @author Artemenko Anton
/// @brief Authenticator interface for PUF fingerprint verification

#ifndef GUID_B8E28E8D_57E6_410C_BC71_C50992BD9B1D
#define GUID_B8E28E8D_57E6_410C_BC71_C50992BD9B1D

#include <fingerprint.hpp>

namespace puf
{

/// @brief Device fingerprint verifier against a reference value
class IAuthenticator
{
   public:
    virtual ~IAuthenticator() = default;

    /// @brief Verifies device authenticity using the presented fingerprint
    /// @param[in] candidate Fingerprint obtained during authentication
    /// @return true if the fingerprint matches the reference within the threshold
    virtual bool Authenticate(const Fingerprint& candidate) = 0;
};

}  // namespace puf

#endif  // GUID_B8E28E8D_57E6_410C_BC71_C50992BD9B1D
