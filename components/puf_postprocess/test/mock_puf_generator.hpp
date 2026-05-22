/// @file mock_puf_generator.hpp
/// @author Artemenko Anton
/// @brief Test stub for IPufGenerator with predefined output

#ifndef GUID_A3F7C201_9B4E_4D83_BE12_0F6D2A8E53C9
#define GUID_A3F7C201_9B4E_4D83_BE12_0F6D2A8E53C9

#include <i_puf_generator.hpp>
#include <vector>

namespace puf::test
{

/// @brief Returns a fixed fingerprint on every call to Generate()
class MockPufGenerator final : public IPufGenerator
{
   public:
    /// @param[in] fp Fingerprint returned on every call to Generate()
    explicit MockPufGenerator(Fingerprint fp) : fp_(std::move(fp))
    {
    }

    Fingerprint Generate() override
    {
        return fp_;
    }

    size_t FingerprintBits() const override
    {
        return fp_.size() * 8;
    }

   private:
    Fingerprint fp_;  ///< Fixed fingerprint
};

/// @brief Returns fingerprints in turn from a given sequence
class MockPufGeneratorSequence final : public IPufGenerator
{
   public:
    /// @param[in] sequence Vector of fingerprints yielded cyclically
    explicit MockPufGeneratorSequence(std::vector<Fingerprint> sequence)
        : sequence_(std::move(sequence)), idx_(0)
    {
    }

    Fingerprint Generate() override
    {
        return sequence_[idx_++ % sequence_.size()];
    }

    size_t FingerprintBits() const override
    {
        return sequence_[0].size() * 8;
    }

   private:
    std::vector<Fingerprint> sequence_;  ///< Cyclic sequence of fingerprints
    size_t idx_;                         ///< Current index in sequence_
};

}  // namespace puf::test

#endif  // GUID_A3F7C201_9B4E_4D83_BE12_0F6D2A8E53C9
