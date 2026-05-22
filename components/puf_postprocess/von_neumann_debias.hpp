/// @file von_neumann_debias.hpp
/// @author Artemenko Anton
/// @brief IPufGenerator decorator — removes bias using the Von Neumann method

#ifndef GUID_3823B7A0_1145_4027_8B0B_1A5662356C69
#define GUID_3823B7A0_1145_4027_8B0B_1A5662356C69

#include <i_puf_generator.hpp>
#include <memory>

namespace puf
{

/// @brief Applies the Von Neumann algorithm to a raw IPufGenerator fingerprint.
/// Algorithm: processes bit pairs — (0,1)→0, (1,0)→1, identical→discard.
/// Output is shorter than input but statistically unbiased.
/// If not enough bits are collected, queries the generator again.
class VonNeumannDebias final : public IPufGenerator
{
   public:
    /// @param[in] inner      Source generator (ownership is transferred)
    /// @param[in] targetBits Desired output fingerprint length in bits
    VonNeumannDebias(std::unique_ptr<IPufGenerator> inner, size_t targetBits);

    /// @brief Applies Von Neumann algorithm to inner_ fingerprint up to targetBits_ bits
    /// @return Unbiased fingerprint of length targetBits_ bits
    Fingerprint Generate() override;

    /// @return Target length of the unbiased fingerprint in bits
    size_t FingerprintBits() const override;

   private:
    std::unique_ptr<IPufGenerator> inner_;  ///< Source generator of the raw fingerprint
    size_t targetBits_;                     ///< Desired output fingerprint length in bits
};

}  // namespace puf

#endif  // GUID_3823B7A0_1145_4027_8B0B_1A5662356C69
