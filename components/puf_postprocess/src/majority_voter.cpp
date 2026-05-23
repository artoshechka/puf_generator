/// @file majority_voter.cpp
/// @author Artemenko Anton
/// @brief Implementation of majority voting

#include <majority_voter.hpp>
#include <cassert>
#include <vector>

namespace puf
{

MajorityVoter::MajorityVoter(std::unique_ptr<IPufGenerator> inner, size_t rounds)
    : inner_(std::move(inner)), rounds_(rounds)
{
    assert(rounds_ % 2U == 1U);
}

size_t MajorityVoter::FingerprintBits() const
{
    return inner_->FingerprintBits();
}

Fingerprint MajorityVoter::Generate()
{
    const size_t bits = inner_->FingerprintBits();
    const size_t bytes = (bits + 7U) / 8U;

    std::vector<size_t> votes(bits, 0U);

    for (size_t r = 0; r < rounds_; ++r)
    {
        const Fingerprint sample = inner_->Generate();
        for (size_t i = 0; i < bits; ++i)
        {
            votes[i] += (sample[i / 8U] >> (i % 8U)) & 1U;
        }
    }

    Fingerprint result(bytes, 0U);
    for (size_t i = 0; i < bits; ++i)
    {
        if (votes[i] > rounds_ / 2U)
        {
            result[i / 8U] |= static_cast<uint8_t>(1U << (i % 8U));
        }
    }

    return result;
}

}  // namespace puf
