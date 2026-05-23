/// @file hamming_authenticator.hpp
/// @author Artemenko Anton
/// @brief Аутентификатор на основе расстояния Хэмминга

#ifndef GUID_F712E89C_625A_4C4E_AC3B_29B337659DF0
#define GUID_F712E89C_625A_4C4E_AC3B_29B337659DF0

#include <i_authenticator.hpp>
#include <puf_auth_config.hpp>

namespace puf
{

/// @brief Сравнивает отпечатки по расстоянию Хэмминга.
/// Признаёт кандидата подлинным, если доля различающихся битов
/// не превышает порог (по умолчанию 10%).
class HammingAuthenticator final : public IAuthenticator
{
   public:
    /// @param[in] reference    Эталонный отпечаток (зарегистрированный)
    /// @param[in] thresholdPct Максимально допустимое внутреннее расстояние Хэмминга в процентах [0..100]
    HammingAuthenticator(Fingerprint reference, double thresholdPct = kDefaultThresholdPct);

    /// @brief Сравнивает кандидата с эталоном по расстоянию Хэмминга
    /// @param[in] candidate Отпечаток, предъявленный при аутентификации
    /// @return true, если относительное расстояние Хэмминга не превышает thresholdPct_/100
    bool Authenticate(const Fingerprint& candidate) override;

    /// @brief Подсчитывает количество различающихся битов между двумя отпечатками
    /// @return Расстояние Хэмминга в битах
    static size_t HammingDistance(const Fingerprint& a, const Fingerprint& b);

    /// @brief Нормализует расстояние Хэмминга по длине отпечатка
    /// @return Относительное расстояние Хэмминга [0.0, 1.0]
    static double FractionalHD(const Fingerprint& a, const Fingerprint& b);

   private:
    Fingerprint reference_;  ///< Эталонный отпечаток, зарегистрированный при инициализации
    double thresholdPct_;    ///< Порог принятия в процентах [0..100]
};

}  // namespace puf

#endif  // GUID_F712E89C_625A_4C4E_AC3B_29B337659DF0
