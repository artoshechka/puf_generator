/// @file puf_type.hpp
/// @author Artemenko Anton
/// @brief Тип источника энтропии PUF

#ifndef GUID_B9F2E4A7_1C38_4D6B_8E05_3A7C9F2D1B84
#define GUID_B9F2E4A7_1C38_4D6B_8E05_3A7C9F2D1B84

namespace puf
{

/// @brief Источник энтропии PUF-генератора
enum class PufType {
    Ro,    ///< Ring Oscillator PUF
    Sram,  ///< SRAM PUF по начальному состоянию памяти
};

}  // namespace puf

#endif  // GUID_B9F2E4A7_1C38_4D6B_8E05_3A7C9F2D1B84
