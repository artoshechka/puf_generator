/// @file ro_oscillator.cpp
/// @author Artemenko Anton
/// @brief Ring Oscillator implementation for ESP32 (Xtensa LX6, ESP-IDF v5.x)

#include <esp_cpu.h>

#include <cassert>
#include <ro_oscillator.hpp>

namespace puf
{

// ─── Oscillators ──────────────────────────────────────────────────────────────
//
// A volatile sink initialized with a unique id guarantees unique machine code
// for each function — the compiler cannot merge them.
// Different IRAM addresses → different cache-line alignment → counter variation.

#define DEFINE_OSC(id)                                                                             \
    static IRAM_ATTR __attribute__((noinline, optimize("O1"))) uint32_t oscFn##id(uint32_t window) \
    {                                                                                              \
        volatile uint32_t sink = (id);                                                             \
        volatile uint32_t cnt = 0U;                                                                \
        const uint32_t t0 = esp_cpu_get_cycle_count();                                             \
        while ((esp_cpu_get_cycle_count() - t0) < window)                                          \
        {                                                                                          \
            const uint32_t tmp = sink ^ cnt;                                                       \
            sink = tmp;                                                                            \
            cnt = cnt + 1U;                                                                        \
        }                                                                                          \
        (void)sink;                                                                                \
        return cnt;                                                                                \
    }

DEFINE_OSC(0)
DEFINE_OSC(1)
DEFINE_OSC(2)
DEFINE_OSC(3)
DEFINE_OSC(4)
DEFINE_OSC(5) DEFINE_OSC(6) DEFINE_OSC(7) DEFINE_OSC(8) DEFINE_OSC(9) DEFINE_OSC(10) DEFINE_OSC(11) DEFINE_OSC(12)
    DEFINE_OSC(13) DEFINE_OSC(14) DEFINE_OSC(15) DEFINE_OSC(16) DEFINE_OSC(17) DEFINE_OSC(18) DEFINE_OSC(19)
        DEFINE_OSC(20) DEFINE_OSC(21) DEFINE_OSC(22) DEFINE_OSC(23) DEFINE_OSC(24) DEFINE_OSC(25) DEFINE_OSC(26)
            DEFINE_OSC(27) DEFINE_OSC(28) DEFINE_OSC(29) DEFINE_OSC(30) DEFINE_OSC(31)

#undef DEFINE_OSC

                using OscFn = uint32_t (*)(uint32_t);

static const OscFn kOscTable[] = {
    oscFn0,  oscFn1,  oscFn2,  oscFn3,  oscFn4,  oscFn5,  oscFn6,  oscFn7,  oscFn8,  oscFn9,  oscFn10,
    oscFn11, oscFn12, oscFn13, oscFn14, oscFn15, oscFn16, oscFn17, oscFn18, oscFn19, oscFn20, oscFn21,
    oscFn22, oscFn23, oscFn24, oscFn25, oscFn26, oscFn27, oscFn28, oscFn29, oscFn30, oscFn31,
};

// ─── RoOscillator ─────────────────────────────────────────────────────────────

RoOscillator::RoOscillator(size_t index) : index_(index)
{
    assert(index <= kMaxIndex);
}

uint32_t RoOscillator::Measure(uint32_t windowCycles)
{
    return kOscTable[index_](windowCycles);
}

}  // namespace puf
