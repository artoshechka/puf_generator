/// @file ipuf_factory.hpp
/// @author Artemenko Anton
/// @brief Interface for a platform-specific PUF generator factory

#ifndef GUID_44869DE5_11BC_4BC6_ACB6_F935000C3080
#define GUID_44869DE5_11BC_4BC6_ACB6_F935000C3080

#include <cstddef>
#include <i_puf_generator.hpp>
#include <memory>
#include <puf_type.hpp>
#include <stdexcept>

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

    /// @brief Создаёт SRAM PUF
    /// @param[in] bits Длина отпечатка в битах
    virtual std::unique_ptr<IPufGenerator> CreateSramPuf(size_t bits) = 0;

    /// @brief Создаёт генератор заданного типа
    /// @param[in] type Тип источника энтропии
    /// @param[in] bits Длина отпечатка в битах
    /// @return Полностью настроенный генератор отпечатков
    virtual std::unique_ptr<IPufGenerator> Create(PufType type, size_t bits)
    {
        switch (type)
        {
            case PufType::Ro:
                return CreateRoPuf(bits);
            case PufType::Sram:
                return CreateSramPuf(bits);
            default:
                throw std::invalid_argument("unknown PUF type");
        }
    }
};

}  // namespace puf

#endif  // GUID_44869DE5_11BC_4BC6_ACB6_F935000C3080
