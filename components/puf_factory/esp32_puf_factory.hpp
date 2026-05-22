/// @file esp32_puf_factory.hpp
/// @author Artemenko Anton
/// @brief Реализация фабрики PUF для ESP32

#ifndef GUID_3F4704FD_47CA_432D_B8AF_5A4886709E85
#define GUID_3F4704FD_47CA_432D_B8AF_5A4886709E85

#include <ipuf_factory.hpp>

namespace puf {

/// @brief Реализация фабрики PUF для платформы ESP32.
///
/// Создаёт RoPuf из 32 программных IRAM-осцилляторов (RoOscillator).
class Esp32PufFactory final : public IPufFactory {
public:
    /// @brief Создаёт RoPuf с минимально необходимым числом осцилляторов
    /// @param bits Желаемая длина отпечатка в битах (≤ N*(N-1)/2, N — осцилляторы)
    /// @return Полностью сконфигурированный генератор отпечатка
    std::unique_ptr<IPufGenerator> CreateRoPuf(size_t bits) override;
};

} // namespace puf

#endif // GUID_3F4704FD_47CA_432D_B8AF_5A4886709E85
