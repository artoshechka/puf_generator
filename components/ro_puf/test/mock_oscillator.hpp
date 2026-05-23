/// @file mock_oscillator.hpp
/// @author Artemenko Anton
/// @brief Тестовая заглушка осциллятора с фиксированным значением счётчика

#ifndef GUID_0C75333C_AD2B_4B27_9E61_678103C88833
#define GUID_0C75333C_AD2B_4B27_9E61_678103C88833

#include <i_oscillator.hpp>

namespace puf::test
{

/// @brief Возвращает фиксированное значение счётчика — используется в тестах RoPuf
class MockOscillator final : public IOscillator
{
   public:
    /// @param[in] fixedCount Значение, которое Measure() будет возвращать для любого окна
    explicit MockOscillator(uint32_t fixedCount) : fixedCount_(fixedCount)
    {
    }

    /// @brief Всегда возвращает fixedCount_ независимо от windowCycles
    /// @return Фиксированное значение счётчика
    uint32_t Measure(uint32_t) override
    {
        return fixedCount_;
    }

   private:
    uint32_t fixedCount_;  ///< Фиксированное значение счётчика для возврата
};

}  // namespace puf::test

#endif  // GUID_0C75333C_AD2B_4B27_9E61_678103C88833
