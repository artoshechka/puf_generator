/// @file ipuf_factory.cpp
/// @brief Реализация дефолтного метода IPufFactory::Create

#include <ipuf_factory.hpp>
#include <stdexcept>

namespace puf
{

std::unique_ptr<IPufGenerator> IPufFactory::Create(PufType type, size_t bits)
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

}  // namespace puf
