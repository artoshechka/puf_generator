/// @file ro_puf.cpp
/// @author Artemenko Anton
/// @brief Реализация Ring Oscillator PUF

#include <ro_puf.hpp>

#include <cassert>

namespace puf {

RoPuf::RoPuf(std::vector<std::unique_ptr<IOscillator>> oscillators,
             size_t bits,
             uint32_t windowCycles)
    : oscillators_(std::move(oscillators))
    , bits_(bits)
    , window_(windowCycles)
{
    assert(oscillators_.size() >= 2);
}

size_t RoPuf::FingerprintBits() const {
    return bits_;
}

Fingerprint RoPuf::Generate() {
    const size_t n = oscillators_.size();
    std::vector<uint32_t> counts(n);

    for (size_t i = 0; i < n; ++i) {
        counts[i] = oscillators_[i]->Measure(window_);
    }

    Fingerprint fp((bits_ + 7) / 8, 0);
    size_t bitIdx = 0;

    // Попарное сравнение: counts[i] > counts[j] → бит 1, иначе 0
    for (size_t i = 0; i < n && bitIdx < bits_; ++i) {
        for (size_t j = i + 1; j < n && bitIdx < bits_; ++j) {
            if (counts[i] > counts[j]) {
                fp[bitIdx / 8] |= static_cast<uint8_t>(1u << (bitIdx % 8));
            }
            ++bitIdx;
        }
    }

    return fp;
}

} // namespace puf
