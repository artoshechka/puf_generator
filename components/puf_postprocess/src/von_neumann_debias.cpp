/// @file von_neumann_debias.cpp
/// @author Artemenko Anton
/// @brief Реализация декоратора фон Неймана

#include <von_neumann_debias.hpp>

namespace puf {

VonNeumannDebias::VonNeumannDebias(std::unique_ptr<IPufGenerator> inner, size_t targetBits)
    : inner_(std::move(inner))
    , targetBits_(targetBits)
{}

size_t VonNeumannDebias::FingerprintBits() const
{
    return targetBits_;
}

Fingerprint VonNeumannDebias::Generate()
{
    Fingerprint result((targetBits_ + 7) / 8, 0);
    size_t outIdx = 0;

    while (outIdx < targetBits_) {
        const Fingerprint raw = inner_->Generate();
        const size_t rawBits  = raw.size() * 8;

        for (size_t i = 0; i + 1 < rawBits && outIdx < targetBits_; i += 2) {
            const uint8_t b0 = (raw[i / 8]     >> (i % 8))     & 1u;
            const uint8_t b1 = (raw[(i+1) / 8] >> ((i+1) % 8)) & 1u;

            if (b0 == b1) continue;  // одинаковые — отброс

            if (b0) {
                result[outIdx / 8] |= static_cast<uint8_t>(1u << (outIdx % 8));
            }
            ++outIdx;
        }
    }

    return result;
}

} // namespace puf
