/// @file hamming_authenticator.hpp
/// @author Artemenko Anton
/// @brief Аутентификатор на основе расстояния Хэмминга

#ifndef GUID_F712E89C_625A_4C4E_AC3B_29B337659DF0
#define GUID_F712E89C_625A_4C4E_AC3B_29B337659DF0

#include <i_authenticator.hpp>

namespace puf {

/// @brief Сравнивает отпечатки по расстоянию Хэмминга.
///
/// Принимает кандидата подлинным, если доля отличающихся битов
/// не превышает порог (по умолчанию 10%).
class HammingAuthenticator final : public IAuthenticator {
public:
    /// @param reference   Эталонный отпечаток (зарегистрированный)
    /// @param thresholdPct Максимально допустимый intra-HD в процентах [0..100]
    HammingAuthenticator(Fingerprint reference, double thresholdPct = 10.0);

    bool Authenticate(const Fingerprint& candidate) override;

    /// @return Расстояние Хэмминга между двумя отпечатками в битах
    static size_t HammingDistance(const Fingerprint& a, const Fingerprint& b);

    /// @return Дробное расстояние Хэмминга [0.0, 1.0]
    static double FractionalHD(const Fingerprint& a, const Fingerprint& b);

private:
    Fingerprint reference_;
    double thresholdPct_;
};

} // namespace puf

#endif // GUID_F712E89C_625A_4C4E_AC3B_29B337659DF0
