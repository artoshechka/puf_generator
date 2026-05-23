/// @file von_neumann_debias.cpp
/// @author Artemenko Anton
/// @brief Implementation of the Von Neumann decorator

#include <stdexcept>
#include <von_neumann_debias.hpp>

namespace puf
{

VonNeumannDebias::VonNeumannDebias(std::unique_ptr<IPufGenerator> inner, size_t targetBits)
    : inner_(std::move(inner)), targetBits_(targetBits)
{
}

size_t VonNeumannDebias::FingerprintBits() const
{
    return targetBits_;
}

Fingerprint VonNeumannDebias::Generate()
{
    Fingerprint result((targetBits_ + 7U) / 8U, 0U);
    size_t outIdx = 0;
    const size_t maxAttempts = targetBits_ * 100U;
    size_t attempts = 0;

    while (outIdx < targetBits_)
    {
        if (attempts++ >= maxAttempts)
        {
            throw std::runtime_error("VonNeumann: insufficient entropy");
        }
        const Fingerprint raw = inner_->Generate();
        const size_t rawBits = raw.size() * 8U;

        for (size_t i = 0; i + 1U < rawBits && outIdx < targetBits_; i += 2U)
        {
            const uint8_t b0 = (raw[i / 8U] >> (i % 8U)) & 1U;
            const uint8_t b1 = (raw[(i + 1U) / 8U] >> ((i + 1U) % 8U)) & 1U;

            if (b0 == b1)
            {
                continue;
            }

            if (b0 != 0U)
            {
                result[outIdx / 8U] |= static_cast<uint8_t>(1U << (outIdx % 8U));
            }
            ++outIdx;
        }
    }

    return result;
}

}  // namespace puf
