/// @file ro_puf.hpp
/// @author Artemenko Anton
/// @brief Ring Oscillator PUF — identifier generator based on oscillators

#ifndef GUID_B783FA41_CE3D_4FE4_AD53_A9238B2C0BAE
#define GUID_B783FA41_CE3D_4FE4_AD53_A9238B2C0BAE

#include <i_oscillator.hpp>
#include <i_puf_generator.hpp>
#include <memory>
#include <ro_puf_config.hpp>
#include <vector>

namespace puf
{

/// @brief PUF generator based on ring oscillators.
/// Takes a set of IOscillator instances and produces a fingerprint by pairwise
/// comparison of counters. Platform-independent — works with any IOscillator implementation.
class RoPuf final : public IPufGenerator
{
   public:
    /// @param[in] oscillators  Set of oscillators, minimum 2
    /// @param[in] bits         Fingerprint length in bits
    /// @param[in] windowCycles Measurement window duration in cycles
    RoPuf(std::vector<std::unique_ptr<IOscillator>> oscillators, size_t bits,
          uint32_t windowCycles = kDefaultWindowCycles);

    /// @brief Generates a fingerprint by pairwise comparison of oscillator counters
    /// @return Byte vector of length ceil(bits/8)
    Fingerprint Generate() override;

    /// @return Fingerprint length in bits as specified at construction
    size_t FingerprintBits() const override;

   private:
    std::vector<std::unique_ptr<IOscillator>> oscillators_;  ///< Set of oscillators
    size_t bits_;                                            ///< Fingerprint length in bits
    uint32_t window_;                                        ///< Measurement window in CPU cycles
};

}  // namespace puf

#endif  // GUID_B783FA41_CE3D_4FE4_AD53_A9238B2C0BAE
