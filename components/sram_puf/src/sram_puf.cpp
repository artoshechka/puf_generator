/// @file sram_puf.cpp
/// @author Artemenko Anton
/// @brief Реализация SRAM PUF

#include <cassert>
#include <sram_puf.hpp>

namespace puf
{

SramPuf::SramPuf(const uint8_t* base, size_t byteCount, size_t bits)
    : base_(base), byteCount_(byteCount), bits_(bits)
{
    assert(base != nullptr);
    assert(byteCount * 8U >= bits);
}

size_t SramPuf::FingerprintBits() const
{
    return bits_;
}

Fingerprint SramPuf::Generate()
{
    const size_t bytes = (bits_ + 7U) / 8U;
    return Fingerprint(base_, base_ + bytes);
}

}  // namespace puf
