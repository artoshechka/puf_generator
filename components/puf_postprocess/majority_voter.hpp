/// @file majority_voter.hpp
/// @author Artemenko Anton
/// @brief Декоратор IPufGenerator — повышает стабильность через мажоритарное голосование

#ifndef GUID_CC8BBC5D_E582_4DB0_BE30_55353A736571
#define GUID_CC8BBC5D_E582_4DB0_BE30_55353A736571

#include <i_puf_generator.hpp>
#include <memory>
#include <puf_postprocess_config.hpp>

namespace puf
{

/// @brief Выполняет N измерений и возвращает побитовое большинство.
/// Каждый бит результирующего отпечатка равен 1, если более половины
/// измерений имеют соответствующий бит равным 1. Снижает внутреннее
/// расстояние Хэмминга ценой кратных rounds запросов к inner_.
class MajorityVoter final : public IPufGenerator
{
   public:
    /// @param[in] inner  Исходный генератор (владение передаётся)
    /// @param[in] rounds Количество измерений (нечётное для однозначного большинства)
    MajorityVoter(std::unique_ptr<IPufGenerator> inner, size_t rounds = kDefaultMajorityRounds);

    /// @brief Выполняет rounds_ измерений и возвращает побитовое большинство
    /// @return Стабилизированный отпечаток той же длины, что и у inner_
    Fingerprint Generate() override;

    /// @return Длина отпечатка в битах (делегирует к inner_)
    size_t FingerprintBits() const override;

   private:
    std::unique_ptr<IPufGenerator> inner_;  ///< Исходный генератор, опрашиваемый rounds_ раз
    size_t rounds_;                         ///< Количество измерений (нечётное для явного большинства)
};

}  // namespace puf

#endif  // GUID_CC8BBC5D_E582_4DB0_BE30_55353A736571
