/// @file majority_voter.hpp
/// @author Artemenko Anton
/// @brief Декоратор IPufGenerator — повышает стабильность голосованием большинства

#ifndef GUID_CC8BBC5D_E582_4DB0_BE30_55353A736571
#define GUID_CC8BBC5D_E582_4DB0_BE30_55353A736571

#include <i_puf_generator.hpp>
#include <memory>

namespace puf
{

/// @brief Снимает N измерений и возвращает побитовое большинство.
/// Каждый бит итогового отпечатка равен 1, если в более чем половине
/// измерений соответствующий бит равен 1. Снижает intra-HD за счёт
/// rounds кратных запросов к inner_.
class MajorityVoter final : public IPufGenerator
{
   public:
    /// @param[in] inner  Исходный генератор (владение передаётся)
    /// @param[in] rounds Число измерений (нечётное для однозначного большинства)
    MajorityVoter(std::unique_ptr<IPufGenerator> inner, size_t rounds = 3);

    /// @brief Снимает rounds_ измерений и возвращает побитовое большинство
    /// @return Стабилизированный отпечаток той же длины, что inner_
    Fingerprint Generate() override;

    /// @return Длина отпечатка в битах (делегирует в inner_)
    size_t FingerprintBits() const override;

   private:
    std::unique_ptr<IPufGenerator> inner_;  ///< Исходный генератор, опрашиваемый rounds_ раз
    size_t rounds_;                         ///< Число измерений (нечётное для чёткого большинства)
};

}  // namespace puf

#endif  // GUID_CC8BBC5D_E582_4DB0_BE30_55353A736571
