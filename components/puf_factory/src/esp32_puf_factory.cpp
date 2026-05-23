/// @file esp32_puf_factory.cpp
/// @author Artemenko Anton
/// @brief Реализация фабрики PUF для ESP32

#include <esp32_puf_factory.hpp>
#include <ro_oscillator.hpp>
#include <ro_puf.hpp>
#include <ro_puf_config.hpp>
#include <sram_puf.hpp>
#include <sram_puf_config.hpp>
#include <stdexcept>

namespace puf
{

namespace
{

static volatile uint8_t s_sram_puf_buf[kDefaultSramPufBytes] __attribute__((section(".noinit")));

}  // namespace

namespace
{

size_t RequiredOscillators(size_t bits)
{
    size_t n = 2;
    while ((n * (n - 1U) / 2U) < bits)
    {
        ++n;
    }
    return n;
}

}  // namespace

std::unique_ptr<IPufGenerator> Esp32PufFactory::CreateRoPuf(size_t bits)
{
    const size_t oscCount = RequiredOscillators(bits);

    if (oscCount > RoOscillator::kMaxIndex + 1U)
    {
        throw std::invalid_argument("requested bits exceed oscillator capacity");
    }

    std::vector<std::unique_ptr<IOscillator>> oscs;
    oscs.reserve(oscCount);
    for (size_t i = 0; i < oscCount; ++i)
    {
        (void)oscs.emplace_back(std::make_unique<RoOscillator>(i));
    }

#ifdef CONFIG_PUF_WINDOW_CYCLES
    const uint32_t windowCycles = CONFIG_PUF_WINDOW_CYCLES;
#else
    const uint32_t windowCycles = kDefaultWindowCycles;
#endif

    return std::make_unique<RoPuf>(std::move(oscs), bits, windowCycles);
}

std::unique_ptr<IPufGenerator> Esp32PufFactory::CreateSramPuf(size_t bits)
{
    if (bits > kDefaultSramPufBytes * 8U)
    {
        throw std::invalid_argument("requested bits exceed SRAM PUF buffer capacity");
    }
    return std::make_unique<SramPuf>(s_sram_puf_buf, kDefaultSramPufBytes, bits);
}

}  // namespace puf
