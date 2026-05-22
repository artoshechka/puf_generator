/// @file von_neumann_debias.hpp
/// @author Artemenko Anton
/// @brief Декоратор IPufGenerator — устраняет смещение по методу фон Неймана

#ifndef GUID_3823B7A0_1145_4027_8B0B_1A5662356C69
#define GUID_3823B7A0_1145_4027_8B0B_1A5662356C69

#include <i_puf_generator.hpp>

#include <memory>

namespace puf {

/// @brief Применяет алгоритм фон Неймана к сырому отпечатку IPufGenerator.
///
/// Алгоритм: обрабатывает пары бит — (0,1)→0, (1,0)→1, одинаковые→отброс.
/// Результат короче исходного, но статистически несмещён.
/// При нехватке бит запрашивает генератор повторно.
class VonNeumannDebias final : public IPufGenerator {
public:
    /// @param inner      Исходный генератор
    /// @param targetBits Желаемая длина выходного отпечатка в битах
    VonNeumannDebias(std::unique_ptr<IPufGenerator> inner, size_t targetBits);

    Fingerprint Generate() override;
    size_t FingerprintBits() const override;

private:
    std::unique_ptr<IPufGenerator> inner_;
    size_t targetBits_;
};

} // namespace puf

#endif // GUID_3823B7A0_1145_4027_8B0B_1A5662356C69
