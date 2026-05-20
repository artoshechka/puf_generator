/// @file ro_puf.hpp
/// @author Artemenko Anton
/// @brief Ring Oscillator PUF — генератор идентификатора на основе осцилляторов

#ifndef GUID_B783FA41_CE3D_4FE4_AD53_A9238B2C0BAE
#define GUID_B783FA41_CE3D_4FE4_AD53_A9238B2C0BAE

#include <i_puf_generator.hpp>
#include <i_oscillator.hpp>

#include <memory>
#include <vector>

namespace puf {

/// @brief Генератор PUF на основе кольцевых осцилляторов.
///
/// Принимает набор IOscillator, формирует отпечаток попарным сравнением
/// счётчиков. Не зависит от платформы — любые реализации IOscillator.
class RoPuf final : public IPufGenerator {
public:
    /// @param oscillators  Набор осцилляторов, минимум 2
    /// @param bits         Длина отпечатка в битах
    /// @param windowCycles Длительность окна измерения в тактах
    RoPuf(std::vector<std::unique_ptr<IOscillator>> oscillators,
          size_t bits,
          uint32_t windowCycles = 200'000);

    Fingerprint Generate() override;
    size_t FingerprintBits() const override;

private:
    std::vector<std::unique_ptr<IOscillator>> oscillators_;
    size_t bits_;
    uint32_t window_;
};

} // namespace puf

#endif // GUID_B783FA41_CE3D_4FE4_AD53_A9238B2C0BAE
