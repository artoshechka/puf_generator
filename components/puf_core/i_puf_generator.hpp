/// @file i_puf_generator.hpp
/// @author Artemenko Anton
/// @brief Интерфейс генератора аппаратного идентификатора

#ifndef GUID_84347C5B_5509_423B_8CAA_92CF9514EFD4
#define GUID_84347C5B_5509_423B_8CAA_92CF9514EFD4

#include <cstddef>
#include <fingerprint.hpp>

namespace puf
{

/// @brief Генератор отпечатка устройства
class IPufGenerator
{
   public:
    virtual ~IPufGenerator() = default;

    /// @brief Генерирует отпечаток устройства
    virtual Fingerprint Generate() = 0;

    /// @return Длина отпечатка в битах
    virtual size_t FingerprintBits() const = 0;
};

}  // namespace puf

#endif  // GUID_84347C5B_5509_423B_8CAA_92CF9514EFD4
