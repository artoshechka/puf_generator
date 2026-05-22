/// @file mock_puf_generator.hpp
/// @author Artemenko Anton
/// @brief Тестовый заменитель IPufGenerator с предопределённым выводом

#ifndef GUID_A3F7C201_9B4E_4D83_BE12_0F6D2A8E53C9
#define GUID_A3F7C201_9B4E_4D83_BE12_0F6D2A8E53C9

#include <i_puf_generator.hpp>
#include <vector>

namespace puf::test
{

/// @brief Возвращает фиксированный отпечаток при каждом вызове Generate()
class MockPufGenerator final : public IPufGenerator
{
   public:
    /// @param[in] fp Отпечаток, возвращаемый при каждом вызове Generate()
    explicit MockPufGenerator(Fingerprint fp) : fp_(std::move(fp))
    {
    }

    Fingerprint Generate() override
    {
        return fp_;
    }

    size_t FingerprintBits() const override
    {
        return fp_.size() * 8;
    }

   private:
    Fingerprint fp_;  ///< Фиксированный отпечаток
};

/// @brief Возвращает отпечатки по очереди из заданной последовательности
class MockPufGeneratorSequence final : public IPufGenerator
{
   public:
    /// @param[in] sequence Вектор отпечатков, выдаваемых циклически
    explicit MockPufGeneratorSequence(std::vector<Fingerprint> sequence)
        : sequence_(std::move(sequence)), idx_(0)
    {
    }

    Fingerprint Generate() override
    {
        return sequence_[idx_++ % sequence_.size()];
    }

    size_t FingerprintBits() const override
    {
        return sequence_[0].size() * 8;
    }

   private:
    std::vector<Fingerprint> sequence_;  ///< Циклическая последовательность отпечатков
    size_t idx_;                         ///< Текущий индекс в sequence_
};

}  // namespace puf::test

#endif  // GUID_A3F7C201_9B4E_4D83_BE12_0F6D2A8E53C9
