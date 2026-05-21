/// @file esp32_puf_factory.hpp
/// @author Artemenko Anton
/// @brief Реализация фабрики PUF для ESP32

#ifndef GUID_3F4704FD_47CA_432D_B8AF_5A4886709E85
#define GUID_3F4704FD_47CA_432D_B8AF_5A4886709E85

#include <ipuf_factory.hpp>

namespace puf {

class Esp32PufFactory final : public IPufFactory {
public:
    std::unique_ptr<IPufGenerator> CreateRoPuf(size_t bits) override;
};

} // namespace puf

#endif // GUID_3F4704FD_47CA_432D_B8AF_5A4886709E85
