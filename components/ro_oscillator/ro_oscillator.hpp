/// @file ro_oscillator.hpp
/// @author Artemenko Anton
/// @brief Ring Oscillator — software oscillator for ESP32

#ifndef GUID_AFD8C96F_D101_414A_A481_CE68B9F27C2E
#define GUID_AFD8C96F_D101_414A_A481_CE68B9F27C2E

#include <cstddef>
#include <i_oscillator.hpp>

namespace puf
{

/// @brief Software ring oscillator placed in IRAM.
/// Frequency variation between instances is caused by differences in IRAM addresses
/// (cache-line alignment, pipeline timing) — a software approximation of RO PUF.
class RoOscillator final : public IOscillator
{
   public:
    static constexpr size_t kMaxIndex = 31;  ///< Maximum allowed index (32 oscillators)

    /// @param[in] index Oscillator index [0, kMaxIndex]
    explicit RoOscillator(size_t index);

    /// @brief Runs a tight loop in IRAM and returns the iteration count
    /// @param[in] windowCycles Window duration in CPU cycles
    /// @return Number of iterations within windowCycles cycles
    uint32_t Measure(uint32_t windowCycles) override;

   private:
    size_t index_;  ///< Oscillator index — determines the IRAM address of the function
};

}  // namespace puf

#endif  // GUID_AFD8C96F_D101_414A_A481_CE68B9F27C2E
