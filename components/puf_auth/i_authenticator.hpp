/// @file i_authenticator.hpp
/// @author Artemenko Anton
/// @brief Интерфейс аутентификатора по PUF-отпечатку

#ifndef GUID_B8E28E8D_57E6_410C_BC71_C50992BD9B1D
#define GUID_B8E28E8D_57E6_410C_BC71_C50992BD9B1D

#include <fingerprint.hpp>

namespace puf {

/// @brief Верификатор отпечатка устройства по эталонному значению
class IAuthenticator {
public:
    virtual ~IAuthenticator() = default;

    /// @brief Проверяет подлинность устройства по предъявленному отпечатку
    /// @param candidate Отпечаток, полученный при аутентификации
    /// @return true если отпечаток соответствует эталону в пределах порога
    virtual bool Authenticate(const Fingerprint& candidate) = 0;
};

} // namespace puf

#endif // GUID_B8E28E8D_57E6_410C_BC71_C50992BD9B1D
