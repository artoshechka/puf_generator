/// @file esp32_puf_factory.cpp
/// @author Artemenko Anton
/// @brief Реализация фабрики PUF для ESP32

#include <esp32_puf_factory.hpp>
#include <ro_oscillator.hpp>
#include <ro_puf.hpp>
#include <stdexcept>

namespace puf
{

namespace
{

/// @brief Минимальное число осцилляторов для получения не менее bits пар
size_t RequiredOscillators(size_t bits)
{
    size_t n = 2;
    while ((n * (n - 1) / 2) < bits) ++n;
    return n;
}

}  // namespace

std::unique_ptr<IPufGenerator> Esp32PufFactory::CreateRoPuf(size_t bits)
{
    const size_t oscCount = RequiredOscillators(bits);

    if (oscCount > RoOscillator::kMaxIndex + 1)
    {
        throw std::invalid_argument("requested bits exceed oscillator capacity");
    }

    std::vector<std::unique_ptr<IOscillator>> oscs;
    oscs.reserve(oscCount);
    for (size_t i = 0; i < oscCount; ++i)
    {
        oscs.emplace_back(std::make_unique<RoOscillator>(i));
    }

    return std::make_unique<RoPuf>(std::move(oscs), bits);
}

}  // namespace puf
