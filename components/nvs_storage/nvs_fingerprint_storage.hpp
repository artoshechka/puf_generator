/// @file nvs_fingerprint_storage.hpp
/// @author Artemenko Anton
/// @brief Fingerprint storage in ESP32 NVS (Non-Volatile Storage)

#ifndef GUID_BF695C74_4626_4E5A_8831_62DB13E155CB
#define GUID_BF695C74_4626_4E5A_8831_62DB13E155CB

#include <i_fingerprint_storage.hpp>

namespace puf
{

/// @brief Stores and loads a fingerprint via the ESP-IDF NVS API.
/// Uses the namespace "puf" and the key "fingerprint".
/// NVS must be initialized before first use: nvs_flash_init().
class NvsFingerprintStorage final : public IFingerprintStorage
{
   public:
    /// @brief Stores the fingerprint in NVS, overwriting any previous value
    /// @param fp Fingerprint to write
    void Store(const Fingerprint& fp) override;

    /// @brief Loads the fingerprint from NVS
    /// @return Previously stored fingerprint
    /// @throws std::runtime_error if the key is not found or NVS is unavailable
    Fingerprint Load() override;

    /// @return true if the key "fingerprint" is present in the NVS namespace "puf"
    bool HasFingerprint() const override;

    /// @brief Deletes the key "fingerprint" from the NVS namespace "puf"
    void Delete() override;

   private:
    static constexpr const char* kNvsNamespace = "puf";    ///< NVS namespace
    static constexpr const char* kNvsKey = "fingerprint";  ///< Fingerprint storage key
};

}  // namespace puf

#endif  // GUID_BF695C74_4626_4E5A_8831_62DB13E155CB
