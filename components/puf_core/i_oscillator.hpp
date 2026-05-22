/// @file i_oscillator.hpp
/// @author Artemenko Anton
/// @brief PUF oscillator interface

#ifndef GUID_2D4A480D_DFF3_430C_AFBB_13695AC0C844
#define GUID_2D4A480D_DFF3_430C_AFBB_13695AC0C844

#include <cstdint>

namespace puf
{

/// @brief Single oscillator — measures iteration count over a given cycle window
class IOscillator
{
   public:
    virtual ~IOscillator() = default;

    /// @param[in] windowCycles Window duration in CPU cycles
    /// @return Number of iterations over windowCycles cycles
    virtual uint32_t Measure(uint32_t windowCycles) = 0;
};

}  // namespace puf

#endif  // GUID_2D4A480D_DFF3_430C_AFBB_13695AC0C844
