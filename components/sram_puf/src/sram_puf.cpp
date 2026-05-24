/// @file sram_puf.cpp
/// @author Artemenko Anton
/// @brief Реализация SRAM PUF

#include <sram_puf.hpp>
#include <stdexcept>

namespace puf
{

SramPuf::SramPuf(const volatile uint8_t* base, size_t byteCount, size_t bits)
    : base_(base), byteCount_(byteCount), bits_(bits)
{
    if (base == nullptr)
    {
        throw std::invalid_argument("SramPuf: base is nullptr");
    }
    if (byteCount * 8U < bits)
    {
        throw std::invalid_argument("SramPuf: byteCount*8 < bits");
    }
}

size_t SramPuf::FingerprintBits() const
{
    return bits_;
}

Fingerprint SramPuf::Generate()
{
    const size_t bytes = (bits_ + 7U) / 8U;
    Fingerprint result(bytes, 0U);
    for (size_t i = 0; i < bytes; ++i)
    {
        result[i] = base_[i];
    }
    return result;
}

}  // namespace puf
