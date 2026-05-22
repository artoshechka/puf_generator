/// @file i_fingerprint_storage.hpp
/// @author Artemenko Anton
/// @brief Интерфейс хранилища эталонного отпечатка

#ifndef GUID_0608B259_B744_40E7_B63C_B7BA37D8125D
#define GUID_0608B259_B744_40E7_B63C_B7BA37D8125D

#include <fingerprint.hpp>

namespace puf
{

/// @brief Хранилище эталонного отпечатка устройства
class IFingerprintStorage
{
   public:
    virtual ~IFingerprintStorage() = default;

    /// @brief Сохраняет отпечаток в энергонезависимую память
    /// @param[in] fp Отпечаток для сохранения
    virtual void Store(const Fingerprint& fp) = 0;

    /// @brief Загружает сохранённый отпечаток
    /// @return Ранее сохранённый отпечаток
    virtual Fingerprint Load() = 0;

    /// @return true если отпечаток уже сохранён
    virtual bool HasFingerprint() const = 0;

    /// @brief Удаляет сохранённый отпечаток из энергонезависимой памяти
    virtual void Delete() = 0;
};

}  // namespace puf

#endif  // GUID_0608B259_B744_40E7_B63C_B7BA37D8125D
