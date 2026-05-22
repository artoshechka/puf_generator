/// @file ipuf_factory.hpp
/// @author Artemenko Anton
/// @brief Интерфейс платформо-специфичной фабрики генераторов PUF

#ifndef GUID_44869DE5_11BC_4BC6_ACB6_F935000C3080
#define GUID_44869DE5_11BC_4BC6_ACB6_F935000C3080

#include <i_puf_generator.hpp>

#include <cstddef>
#include <memory>

namespace puf {

/// @brief Фабрика генераторов PUF для конкретной платформы
class IPufFactory {
public:
    virtual ~IPufFactory() = default;

    /// @brief Создаёт Ring Oscillator PUF
    /// @param[in] bits Длина отпечатка в битах
    virtual std::unique_ptr<IPufGenerator> CreateRoPuf(size_t bits) = 0;
};

} // namespace puf

#endif // GUID_44869DE5_11BC_4BC6_ACB6_F935000C3080
