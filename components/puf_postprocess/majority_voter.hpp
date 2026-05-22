/// @file majority_voter.hpp
/// @author Artemenko Anton
/// @brief IPufGenerator decorator — improves stability via majority voting

#ifndef GUID_CC8BBC5D_E582_4DB0_BE30_55353A736571
#define GUID_CC8BBC5D_E582_4DB0_BE30_55353A736571

#include <i_puf_generator.hpp>
#include <memory>
#include <puf_postprocess_config.hpp>

namespace puf
{

/// @brief Takes N measurements and returns the bitwise majority.
/// Each bit of the resulting fingerprint is 1 if more than half of the
/// measurements have the corresponding bit equal to 1. Reduces intra-HD
/// at the cost of rounds multiple queries to inner_.
class MajorityVoter final : public IPufGenerator
{
   public:
    /// @param[in] inner  Source generator (ownership is transferred)
    /// @param[in] rounds Number of measurements (odd for unambiguous majority)
    MajorityVoter(std::unique_ptr<IPufGenerator> inner, size_t rounds = kDefaultMajorityRounds);

    /// @brief Takes rounds_ measurements and returns the bitwise majority
    /// @return Stabilized fingerprint of the same length as inner_
    Fingerprint Generate() override;

    /// @return Fingerprint length in bits (delegates to inner_)
    size_t FingerprintBits() const override;

   private:
    std::unique_ptr<IPufGenerator> inner_;  ///< Source generator queried rounds_ times
    size_t rounds_;                         ///< Number of measurements (odd for clear majority)
};

}  // namespace puf

#endif  // GUID_CC8BBC5D_E582_4DB0_BE30_55353A736571
