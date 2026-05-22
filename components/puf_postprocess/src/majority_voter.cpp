/// @file majority_voter.cpp
/// @author Artemenko Anton
/// @brief Реализация голосования большинством

#include <majority_voter.hpp>

#include <vector>

namespace puf {

MajorityVoter::MajorityVoter(std::unique_ptr<IPufGenerator> inner, size_t rounds)
    : inner_(std::move(inner))
    , rounds_(rounds)
{}

size_t MajorityVoter::FingerprintBits() const
{
    return inner_->FingerprintBits();
}

Fingerprint MajorityVoter::Generate()
{
    const size_t bits  = inner_->FingerprintBits();
    const size_t bytes = (bits + 7) / 8;

    std::vector<size_t> votes(bits, 0);

    for (size_t r = 0; r < rounds_; ++r) {
        const Fingerprint sample = inner_->Generate();
        for (size_t i = 0; i < bits; ++i) {
            votes[i] += (sample[i / 8] >> (i % 8)) & 1u;
        }
    }

    Fingerprint result(bytes, 0);
    for (size_t i = 0; i < bits; ++i) {
        if (votes[i] > rounds_ / 2) {
            result[i / 8] |= static_cast<uint8_t>(1u << (i % 8));
        }
    }

    return result;
}

} // namespace puf
