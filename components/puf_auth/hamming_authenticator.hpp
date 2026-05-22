/// @file hamming_authenticator.hpp
/// @author Artemenko Anton
/// @brief Аутентификатор на основе расстояния Хэмминга

#ifndef GUID_F712E89C_625A_4C4E_AC3B_29B337659DF0
#define GUID_F712E89C_625A_4C4E_AC3B_29B337659DF0

#include <i_authenticator.hpp>

namespace puf
{

/// @brief Сравнивает отпечатки по расстоянию Хэмминга.
/// Принимает кандидата подлинным, если доля отличающихся битов
/// не превышает порог (по умолчанию 10%).
class HammingAuthenticator final : public IAuthenticator
{
   public:
    /// @param[in] reference    Эталонный отпечаток (зарегистрированный)
    /// @param[in] thresholdPct Максимально допустимый intra-HD в процентах [0..100]
    HammingAuthenticator(Fingerprint reference, double thresholdPct = 10.0);

    /// @brief Сравнивает candidate с эталоном по расстоянию Хэмминга
    /// @param[in] candidate Отпечаток, предъявленный при аутентификации
    /// @return true если дробное HD не превышает thresholdPct_/100
    bool Authenticate(const Fingerprint& candidate) override;

    /// @brief Считает число различающихся бит между двумя отпечатками
    /// @return Расстояние Хэмминга в битах
    static size_t HammingDistance(const Fingerprint& a, const Fingerprint& b);

    /// @brief Нормирует расстояние Хэмминга на длину отпечатка
    /// @return Дробное расстояние Хэмминга [0.0, 1.0]
    static double FractionalHD(const Fingerprint& a, const Fingerprint& b);

   private:
    Fingerprint reference_;  ///< Эталонный отпечаток, зарегистрированный при энролменте
    double thresholdPct_;    ///< Порог приёма в процентах [0..100]
};

}  // namespace puf

#endif  // GUID_F712E89C_625A_4C4E_AC3B_29B337659DF0
