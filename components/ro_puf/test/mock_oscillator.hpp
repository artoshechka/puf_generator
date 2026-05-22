/// @file mock_oscillator.hpp
/// @author Artemenko Anton
/// @brief Test stub oscillator with a fixed counter value

#ifndef GUID_0C75333C_AD2B_4B27_9E61_678103C88833
#define GUID_0C75333C_AD2B_4B27_9E61_678103C88833

#include <i_oscillator.hpp>

namespace puf::test
{

/// @brief Returns a fixed counter value — used in RoPuf tests
class MockOscillator final : public IOscillator
{
   public:
    /// @param[in] fixedCount Value that Measure() will return for any window
    explicit MockOscillator(uint32_t fixedCount) : fixedCount_(fixedCount)
    {
    }

    /// @brief Always returns fixedCount_ regardless of windowCycles
    /// @return Fixed counter value
    uint32_t Measure(uint32_t) override
    {
        return fixedCount_;
    }

   private:
    uint32_t fixedCount_;  ///< Fixed counter value to return
};

}  // namespace puf::test

#endif  // GUID_0C75333C_AD2B_4B27_9E61_678103C88833
