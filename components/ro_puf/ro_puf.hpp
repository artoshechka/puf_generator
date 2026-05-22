/// @file ro_puf.hpp
/// @author Artemenko Anton
/// @brief Ring Oscillator PUF — генератор идентификатора на основе осцилляторов

#ifndef GUID_B783FA41_CE3D_4FE4_AD53_A9238B2C0BAE
#define GUID_B783FA41_CE3D_4FE4_AD53_A9238B2C0BAE

#include <i_oscillator.hpp>
#include <i_puf_generator.hpp>
#include <memory>
#include <ro_puf_config.hpp>
#include <vector>

namespace puf
{

/// @brief Генератор PUF на основе кольцевых осцилляторов.
/// Принимает набор IOscillator, формирует отпечаток попарным сравнением
/// счётчиков. Не зависит от платформы — любые реализации IOscillator.
class RoPuf final : public IPufGenerator
{
   public:
    /// @param[in] oscillators  Набор осцилляторов, минимум 2
    /// @param[in] bits         Длина отпечатка в битах
    /// @param[in] windowCycles Длительность окна измерения в тактах
    RoPuf(std::vector<std::unique_ptr<IOscillator>> oscillators, size_t bits,
          uint32_t windowCycles = kDefaultWindowCycles);

    /// @brief Генерирует отпечаток попарным сравнением счётчиков осцилляторов
    /// @return Вектор байт длиной ceil(bits/8)
    Fingerprint Generate() override;

    /// @return Длина отпечатка в битах, заданная при конструировании
    size_t FingerprintBits() const override;

   private:
    std::vector<std::unique_ptr<IOscillator>> oscillators_;  ///< Набор осцилляторов
    size_t bits_;                                            ///< Длина отпечатка в битах
    uint32_t window_;                                        ///< Окно измерения в тактах процессора
};

}  // namespace puf

#endif  // GUID_B783FA41_CE3D_4FE4_AD53_A9238B2C0BAE
