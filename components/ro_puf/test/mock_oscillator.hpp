/// @file mock_oscillator.hpp
/// @author Artemenko Anton
/// @brief Тестовый заменитель осциллятора с фиксированным счётчиком

#ifndef GUID_0C75333C_AD2B_4B27_9E61_678103C88833
#define GUID_0C75333C_AD2B_4B27_9E61_678103C88833

#include <i_oscillator.hpp>

namespace puf::test
{

/// @brief Возвращает фиксированное значение счётчика — используется в тестах RoPuf
class MockOscillator final : public IOscillator
{
   public:
    /// @param[in] fixedCount Значение, которое будет возвращать Measure() при любом окне
    explicit MockOscillator(uint32_t fixedCount) : fixedCount_(fixedCount)
    {
    }

    /// @brief Всегда возвращает fixedCount_ независимо от windowCycles
    /// @return Фиксированный счётчик
    uint32_t Measure(uint32_t) override
    {
        return fixedCount_;
    }

   private:
    uint32_t fixedCount_;  ///< Фиксированное возвращаемое значение счётчика
};

}  // namespace puf::test

#endif  // GUID_0C75333C_AD2B_4B27_9E61_678103C88833
