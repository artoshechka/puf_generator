/// @file i_fingerprint_storage.hpp
/// @author Artemenko Anton
/// @brief Interface for reference fingerprint storage

#ifndef GUID_0608B259_B744_40E7_B63C_B7BA37D8125D
#define GUID_0608B259_B744_40E7_B63C_B7BA37D8125D

#include <fingerprint.hpp>

namespace puf
{

/// @brief Storage for the device reference fingerprint
class IFingerprintStorage
{
   public:
    virtual ~IFingerprintStorage() = default;

    /// @brief Stores the fingerprint in non-volatile memory
    /// @param[in] fp Fingerprint to store
    virtual void Store(const Fingerprint& fp) = 0;

    /// @brief Loads the stored fingerprint
    /// @return Previously stored fingerprint
    virtual Fingerprint Load() = 0;

    /// @return true if a fingerprint has already been stored
    virtual bool HasFingerprint() const = 0;

    /// @brief Deletes the stored fingerprint from non-volatile memory
    virtual void Delete() = 0;
};

}  // namespace puf

#endif  // GUID_0608B259_B744_40E7_B63C_B7BA37D8125D
