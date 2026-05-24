/// @file esp32_puf_factory.cpp
/// @author Artemenko Anton
/// @brief Реализация фабрики PUF для ESP32

#include <esp32_puf_factory.hpp>
#include <sram_puf.hpp>
#include <sram_puf_config.hpp>
#include <stdexcept>

namespace puf
{

namespace
{

static volatile uint8_t s_sram_puf_buf[kDefaultSramPufBytes] __attribute__((section(".noinit")));

}  // namespace

std::unique_ptr<IPufGenerator> Esp32PufFactory::CreateSramPuf(size_t bits)
{
    if (bits == 0U)
    {
        throw std::invalid_argument("bits must be > 0");
    }
    if (bits > kDefaultSramPufBytes * 8U)
    {
        throw std::invalid_argument("requested bits exceed SRAM PUF buffer capacity");
    }
    return std::make_unique<SramPuf>(s_sram_puf_buf, kDefaultSramPufBytes, bits);
}

}  // namespace puf
