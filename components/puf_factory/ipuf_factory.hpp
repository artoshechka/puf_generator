/// @file ipuf_factory.hpp
/// @author Artemenko Anton
/// @brief Interface for a platform-specific PUF generator factory

#ifndef GUID_44869DE5_11BC_4BC6_ACB6_F935000C3080
#define GUID_44869DE5_11BC_4BC6_ACB6_F935000C3080

#include <cstddef>
#include <i_puf_generator.hpp>
#include <memory>

namespace puf
{

/// @brief PUF generator factory for a specific platform
class IPufFactory
{
   public:
    virtual ~IPufFactory() = default;

    /// @brief Creates a Ring Oscillator PUF
    /// @param[in] bits Fingerprint length in bits
    virtual std::unique_ptr<IPufGenerator> CreateRoPuf(size_t bits) = 0;
};

}  // namespace puf

#endif  // GUID_44869DE5_11BC_4BC6_ACB6_F935000C3080
