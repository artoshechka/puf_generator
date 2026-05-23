/// @file ro_oscillator.hpp
/// @author Artemenko Anton
/// @brief Ring Oscillator — программный осциллятор для ESP32

#ifndef GUID_AFD8C96F_D101_414A_A481_CE68B9F27C2E
#define GUID_AFD8C96F_D101_414A_A481_CE68B9F27C2E

#include <cstddef>
#include <i_oscillator.hpp>

namespace puf
{

/// @brief Программный кольцевой осциллятор, размещённый в IRAM.
/// Вариация частоты между экземплярами вызвана различиями в адресах IRAM
/// (выравнивание по кэш-линии, тайминги конвейера) — программная аппроксимация RO PUF.
class RoOscillator final : public IOscillator
{
   public:
    static constexpr size_t kMaxIndex = 31;  ///< Максимально допустимый индекс (32 осциллятора)

    /// @param[in] index Индекс осциллятора [0, kMaxIndex]
    explicit RoOscillator(size_t index);

    /// @brief Запускает плотный цикл в IRAM и возвращает количество итераций
    /// @param[in] windowCycles Длительность окна в тактах CPU
    /// @return Количество итераций за windowCycles тактов
    uint32_t Measure(uint32_t windowCycles) override;

   private:
    size_t index_;  ///< Индекс осциллятора — определяет адрес функции в IRAM
};

}  // namespace puf

#endif  // GUID_AFD8C96F_D101_414A_A481_CE68B9F27C2E
