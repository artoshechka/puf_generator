/// @file esp32_puf_factory.hpp
/// @author Artemenko Anton
/// @brief PUF factory implementation for ESP32

#ifndef GUID_3F4704FD_47CA_432D_B8AF_5A4886709E85
#define GUID_3F4704FD_47CA_432D_B8AF_5A4886709E85

#include <ipuf_factory.hpp>

namespace puf
{

/// @brief PUF factory implementation for the ESP32 platform.
class Esp32PufFactory final : public IPufFactory
{
   public:
    /// @brief Создаёт SramPuf на основе буфера в секции .noinit (DRAM)
    /// @param[in] bits Желаемая длина отпечатка в битах (<= kDefaultSramPufBytes * 8)
    /// @return Полностью настроенный генератор отпечатков
    std::unique_ptr<IPufGenerator> CreateSramPuf(size_t bits) override;
};

}  // namespace puf

#endif  // GUID_3F4704FD_47CA_432D_B8AF_5A4886709E85
