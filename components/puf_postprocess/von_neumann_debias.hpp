/// @file von_neumann_debias.hpp
/// @author Artemenko Anton
/// @brief Декоратор IPufGenerator — устраняет смещение методом фон Неймана

#ifndef GUID_3823B7A0_1145_4027_8B0B_1A5662356C69
#define GUID_3823B7A0_1145_4027_8B0B_1A5662356C69

#include <i_puf_generator.hpp>
#include <memory>

namespace puf
{

/// @brief Применяет алгоритм фон Неймана к сырому отпечатку IPufGenerator.
/// Алгоритм: обрабатывает пары битов — (0,1)→0, (1,0)→1, одинаковые→отбрасываются.
/// Выход короче входа, но статистически несмещён.
/// Если собрано недостаточно битов, повторно опрашивает генератор.
class VonNeumannDebias final : public IPufGenerator
{
   public:
    /// @param[in] inner      Исходный генератор (владение передаётся)
    /// @param[in] targetBits Желаемая длина выходного отпечатка в битах
    VonNeumannDebias(std::unique_ptr<IPufGenerator> inner, size_t targetBits);

    /// @brief Применяет алгоритм фон Неймана к отпечатку inner_ до targetBits_ битов
    /// @return Несмещённый отпечаток длиной targetBits_ битов
    Fingerprint Generate() override;

    /// @return Целевая длина несмещённого отпечатка в битах
    size_t FingerprintBits() const override;

   private:
    std::unique_ptr<IPufGenerator> inner_;  ///< Исходный генератор сырого отпечатка
    size_t targetBits_;                     ///< Желаемая длина выходного отпечатка в битах
};

}  // namespace puf

#endif  // GUID_3823B7A0_1145_4027_8B0B_1A5662356C69
