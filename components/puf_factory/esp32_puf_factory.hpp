/// @file esp32_puf_factory.hpp
/// @author Artemenko Anton
/// @brief PUF factory implementation for ESP32

#ifndef GUID_3F4704FD_47CA_432D_B8AF_5A4886709E85
#define GUID_3F4704FD_47CA_432D_B8AF_5A4886709E85

#include <ipuf_factory.hpp>

namespace puf
{

/// @brief PUF factory implementation for the ESP32 platform.
/// Creates a RoPuf from 32 software IRAM oscillators (RoOscillator).
class Esp32PufFactory final : public IPufFactory
{
   public:
    /// @brief Creates a RoPuf with the minimum required number of oscillators
    /// @param[in] bits Desired fingerprint length in bits (≤ N*(N-1)/2, N — oscillators)
    /// @return Fully configured fingerprint generator
    std::unique_ptr<IPufGenerator> CreateRoPuf(size_t bits) override;
};

}  // namespace puf

#endif  // GUID_3F4704FD_47CA_432D_B8AF_5A4886709E85
