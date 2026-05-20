/// @file ro_oscillator.hpp
/// @author Artemenko Anton
/// @brief Ring Oscillator — программный осциллятор для ESP32

#ifndef GUID_AFD8C96F_D101_414A_A481_CE68B9F27C2E
#define GUID_AFD8C96F_D101_414A_A481_CE68B9F27C2E

#include <i_oscillator.hpp>

#include <cstddef>

namespace puf {

/// @brief Программный кольцевой осциллятор, размещённый в IRAM.
///
/// Вариация частот между экземплярами обусловлена разницей IRAM-адресов
/// (cache-line alignment, pipeline timing) — программная аппроксимация RO PUF.
class RoOscillator final : public IOscillator {
public:
    static constexpr size_t kMaxIndex = 31;

    /// @param index Индекс осциллятора [0, kMaxIndex]
    explicit RoOscillator(size_t index);

    uint32_t Measure(uint32_t windowCycles) override;

private:
    size_t index_;
};

} // namespace puf

#endif // GUID_AFD8C96F_D101_414A_A481_CE68B9F27C2E
