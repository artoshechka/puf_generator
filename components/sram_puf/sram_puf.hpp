/// @file sram_puf.hpp
/// @author Artemenko Anton
/// @brief SRAM PUF — генератор по начальному состоянию SRAM при включении питания

#ifndef GUID_E1A53D78_2C94_4B6F_A017_8E3F5C9B2D46
#define GUID_E1A53D78_2C94_4B6F_A017_8E3F5C9B2D46

#include <cstddef>
#include <cstdint>
#include <i_puf_generator.hpp>

namespace puf
{

/// @brief Генератор PUF на основе начального состояния SRAM.
/// Считывает сырые байты из выделенной области памяти и возвращает их как отпечаток.
/// Область должна располагаться в секции .noinit, чтобы сохранять состояние при включении питания.
class SramPuf final : public IPufGenerator
{
   public:
    /// @param[in] base      Указатель на область SRAM с энтропией при включении питания
    /// @param[in] byteCount Количество доступных байт в области
    /// @param[in] bits      Желаемая длина отпечатка в битах (<= byteCount * 8)
    SramPuf(const uint8_t* base, size_t byteCount, size_t bits);

    /// @return Байты отпечатка, считанные напрямую из области SRAM
    Fingerprint Generate() override;

    /// @return Длина отпечатка в битах, заданная при конструировании
    size_t FingerprintBits() const override;

   private:
    const uint8_t* base_;
    size_t byteCount_;
    size_t bits_;
};

}  // namespace puf

#endif  // GUID_E1A53D78_2C94_4B6F_A017_8E3F5C9B2D46
