/// @file nvs_fingerprint_storage.hpp
/// @author Artemenko Anton
/// @brief Хранение отпечатка в ESP32 NVS (Non-Volatile Storage)

#ifndef GUID_BF695C74_4626_4E5A_8831_62DB13E155CB
#define GUID_BF695C74_4626_4E5A_8831_62DB13E155CB

#include <i_fingerprint_storage.hpp>

namespace puf {

/// @brief Сохраняет и загружает отпечаток через ESP-IDF NVS API.
///
/// Использует пространство имён "puf" и ключ "fingerprint".
/// Перед первым использованием необходимо инициализировать NVS: nvs_flash_init().
class NvsFingerprintStorage final : public IFingerprintStorage {
public:
    /// @brief Сохраняет отпечаток в NVS, перезаписывая предыдущее значение
    /// @param fp Отпечаток для записи
    void Store(const Fingerprint& fp) override;

    /// @brief Загружает отпечаток из NVS
    /// @return Ранее сохранённый отпечаток
    /// @throws std::runtime_error если отпечаток не найден
    Fingerprint Load() override;

    /// @return true если ключ "fingerprint" присутствует в NVS-пространстве "puf"
    bool HasFingerprint() const override;

private:
    static constexpr const char* kNvsNamespace = "puf";         ///< NVS-пространство имён
    static constexpr const char* kNvsKey       = "fingerprint"; ///< Ключ хранения отпечатка
};

} // namespace puf

#endif // GUID_BF695C74_4626_4E5A_8831_62DB13E155CB
