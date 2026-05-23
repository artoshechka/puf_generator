/// @file main.cpp
/// @brief Точка входа: генерация PUF-отпечатка и командный интерфейс по UART.
///
/// Поддерживаемые команды (отправить строку + '\n' в монитор):
///   PUF   — сгенерировать и вывести новый отпечаток
///   LOGS  — дамп накопленных логов и очистка буфера
///   RAW   — вывести сырые счётчики всех 32 осцилляторов (3 прогона)

#include <freertos/FreeRTOS.h>
#include <freertos/task.h>
#include <sdkconfig.h>

#include <cstdio>
#include <cstring>
#include <esp32_puf_factory.hpp>
#include <majority_voter.hpp>
#include <puf_log.hpp>
#include <puf_type.hpp>
#include <ro_oscillator.hpp>

namespace
{

constexpr size_t kRawRuns = 3U;
constexpr size_t kOscCount = puf::RoOscillator::kMaxIndex + 1U;

void printFingerprint(const puf::Fingerprint& fp)
{
#ifdef CONFIG_PUF_ALLOW_FINGERPRINT_OUTPUT
    for (const uint8_t byte : fp)
    {
        printf("%02x", byte);
    }
    printf("\n");
    (void)fflush(stdout);
#else
    printf("PUF_OK\n");
    (void)fflush(stdout);
#endif
}

puf::Fingerprint generate()
{
    puf::Esp32PufFactory factory;

#ifdef CONFIG_PUF_TYPE_SRAM
    constexpr puf::PufType kPufType = puf::PufType::Sram;
#else
    constexpr puf::PufType kPufType = puf::PufType::Ro;
#endif

    auto raw = factory.Create(kPufType, CONFIG_PUF_FINGERPRINT_BITS);
    puf::MajorityVoter generator(std::move(raw), CONFIG_PUF_MAJORITY_ROUNDS);
    return generator.Generate();
}

void printRawCounts()
{
    puf::RoOscillator oscs[kOscCount] = {
        puf::RoOscillator(0U),  puf::RoOscillator(1U),  puf::RoOscillator(2U),  puf::RoOscillator(3U),
        puf::RoOscillator(4U),  puf::RoOscillator(5U),  puf::RoOscillator(6U),  puf::RoOscillator(7U),
        puf::RoOscillator(8U),  puf::RoOscillator(9U),  puf::RoOscillator(10U), puf::RoOscillator(11U),
        puf::RoOscillator(12U), puf::RoOscillator(13U), puf::RoOscillator(14U), puf::RoOscillator(15U),
        puf::RoOscillator(16U), puf::RoOscillator(17U), puf::RoOscillator(18U), puf::RoOscillator(19U),
        puf::RoOscillator(20U), puf::RoOscillator(21U), puf::RoOscillator(22U), puf::RoOscillator(23U),
        puf::RoOscillator(24U), puf::RoOscillator(25U), puf::RoOscillator(26U), puf::RoOscillator(27U),
        puf::RoOscillator(28U), puf::RoOscillator(29U), puf::RoOscillator(30U), puf::RoOscillator(31U),
    };

    printf("RAW_BEGIN window=%u\n", static_cast<unsigned>(CONFIG_PUF_WINDOW_CYCLES));
    for (size_t run = 0; run < kRawRuns; ++run)
    {
        printf("RUN %zu:", run);
        for (size_t i = 0; i < kOscCount; ++i)
        {
            printf(" %lu", static_cast<unsigned long>(oscs[i].Measure(CONFIG_PUF_WINDOW_CYCLES)));
        }
        printf("\n");
    }
    printf("RAW_END\n");
    (void)fflush(stdout);
}

}  // namespace

extern "C" void app_main()
{
    puf::LogInit();

    try
    {
        printFingerprint(generate());
    } catch (...)
    {
        // Generation failure must not crash the device; boot continues without fingerprint output.
    }

    char line[32];
    size_t pos = 0;
    while (true)
    {
        const int c = getchar();
        if (c == EOF)
        {
            vTaskDelay(pdMS_TO_TICKS(1));
            continue;
        }

        if (c == '\n' || c == '\r')
        {
            line[pos] = '\0';
            if (strcmp(line, "PUF") == 0)
            {
                try
                {
                    printFingerprint(generate());
                } catch (...)
                {
                    // Generation failure: non-fatal.
                }
            } else if (strcmp(line, "LOGS") == 0)
            {
                puf::LogDump();
            } else if (strcmp(line, "RAW") == 0)
            {
                printRawCounts();
            } else
            {
                // Unknown command — ignore.
            }
            pos = 0;
        } else if (pos < sizeof(line) - 1U)
        {
            line[pos] = static_cast<char>(c);
            ++pos;
        } else
        {
            // Line too long — ignore overflow character.
        }
    }
}
