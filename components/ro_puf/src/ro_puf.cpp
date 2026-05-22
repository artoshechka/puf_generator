/// @file ro_puf.cpp
/// @author Artemenko Anton
/// @brief Ring Oscillator PUF implementation

#include <cassert>
#include <ro_puf.hpp>

namespace puf
{

RoPuf::RoPuf(std::vector<std::unique_ptr<IOscillator>> oscillators, size_t bits, uint32_t windowCycles)
    : oscillators_(std::move(oscillators)), bits_(bits), window_(windowCycles)
{
    assert(oscillators_.size() >= 2U);
}

size_t RoPuf::FingerprintBits() const
{
    return bits_;
}

Fingerprint RoPuf::Generate()
{
    const size_t n = oscillators_.size();
    std::vector<uint32_t> counts(n);

    for (size_t i = 0; i < n; ++i)
    {
        counts[i] = oscillators_[i]->Measure(window_);
    }

    Fingerprint fp((bits_ + 7U) / 8U, 0U);
    size_t bitIdx = 0;

    for (size_t i = 0; i < n && bitIdx < bits_; ++i)
    {
        for (size_t j = i + 1U; j < n && bitIdx < bits_; ++j)
        {
            if (counts[i] > counts[j])
            {
                fp[bitIdx / 8U] |= static_cast<uint8_t>(1U << (bitIdx % 8U));
            }
            ++bitIdx;
        }
    }

    return fp;
}

}  // namespace puf
