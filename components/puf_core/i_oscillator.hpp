/// @file i_oscillator.hpp
/// @author Artemenko Anton
/// @brief Интерфейс осциллятора PUF

#ifndef GUID_2D4A480D_DFF3_430C_AFBB_13695AC0C844
#define GUID_2D4A480D_DFF3_430C_AFBB_13695AC0C844

#include <cstdint>

namespace puf {

/// @brief Один осциллятор — измеряет число итераций за заданное окно тактов
class IOscillator {
public:
    virtual ~IOscillator() = default;

    /// @param windowCycles Длительность окна в тактах процессора
    /// @return Число итераций за windowCycles тактов
    virtual uint32_t Measure(uint32_t windowCycles) = 0;
};

} // namespace puf

#endif // GUID_2D4A480D_DFF3_430C_AFBB_13695AC0C844
