/// @file i_puf_generator.hpp
/// @author Artemenko Anton
/// @brief Hardware identifier generator interface

#ifndef GUID_84347C5B_5509_423B_8CAA_92CF9514EFD4
#define GUID_84347C5B_5509_423B_8CAA_92CF9514EFD4

#include <cstddef>
#include <fingerprint.hpp>

namespace puf
{

/// @brief Device fingerprint generator
class IPufGenerator
{
   public:
    virtual ~IPufGenerator() = default;

    /// @brief Generates the device fingerprint
    virtual Fingerprint Generate() = 0;

    /// @return Fingerprint length in bits
    virtual size_t FingerprintBits() const = 0;
};

}  // namespace puf

#endif  // GUID_84347C5B_5509_423B_8CAA_92CF9514EFD4
