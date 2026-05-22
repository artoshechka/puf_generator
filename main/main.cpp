/// @file main.cpp
/// @brief Точка входа: генерация PUF-отпечатка и командный интерфейс по UART.
///
/// Поддерживаемые команды (отправить строку + '\n' в монитор):
///   PUF   — сгенерировать и вывести новый отпечаток (кешируется в NVS)
///   LOGS  — дамп накопленных логов и очистка буфера
///   DEL   — удалить кешированный отпечаток из NVS
///   RAW   — вывести сырые счётчики всех 32 осцилляторов (3 прогона)

#include <freertos/FreeRTOS.h>
#include <freertos/task.h>
#include <nvs_flash.h>
#include <sdkconfig.h>

#include <cstdio>
#include <cstring>
#include <esp32_puf_factory.hpp>
#include <majority_voter.hpp>
#include <nvs_fingerprint_storage.hpp>
#include <puf_log.hpp>
#include <ro_oscillator.hpp>

static constexpr size_t kRawRuns = 3;
static constexpr size_t kOscCount = puf::RoOscillator::kMaxIndex + 1;

static void printFingerprint(const puf::Fingerprint& fp)
{
    for (const uint8_t byte : fp) printf("%02x", byte);
    printf("\n");
    fflush(stdout);
}

static puf::Fingerprint generateAndStore(puf::NvsFingerprintStorage& storage)
{
    puf::Esp32PufFactory factory;
    auto raw = factory.CreateRoPuf(CONFIG_PUF_FINGERPRINT_BITS);
    puf::MajorityVoter generator(std::move(raw), CONFIG_PUF_MAJORITY_ROUNDS);

    const puf::Fingerprint fp = generator.Generate();
    storage.Store(fp);
    return fp;
}

static void printRawCounts()
{
    puf::RoOscillator oscs[kOscCount] = {
        puf::RoOscillator(0),  puf::RoOscillator(1),  puf::RoOscillator(2),
        puf::RoOscillator(3),  puf::RoOscillator(4),  puf::RoOscillator(5),
        puf::RoOscillator(6),  puf::RoOscillator(7),  puf::RoOscillator(8),
        puf::RoOscillator(9),  puf::RoOscillator(10), puf::RoOscillator(11),
        puf::RoOscillator(12), puf::RoOscillator(13), puf::RoOscillator(14),
        puf::RoOscillator(15), puf::RoOscillator(16), puf::RoOscillator(17),
        puf::RoOscillator(18), puf::RoOscillator(19), puf::RoOscillator(20),
        puf::RoOscillator(21), puf::RoOscillator(22), puf::RoOscillator(23),
        puf::RoOscillator(24), puf::RoOscillator(25), puf::RoOscillator(26),
        puf::RoOscillator(27), puf::RoOscillator(28), puf::RoOscillator(29),
        puf::RoOscillator(30), puf::RoOscillator(31),
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
    fflush(stdout);
}

static void initNvs()
{
    esp_err_t err = nvs_flash_init();
    if (err == ESP_ERR_NVS_NO_FREE_PAGES || err == ESP_ERR_NVS_NEW_VERSION_FOUND)
    {
        nvs_flash_erase();
        nvs_flash_init();
    }
}

extern "C" void app_main()
{
    puf::LogInit();
    initNvs();

    puf::NvsFingerprintStorage storage;

    try
    {
        if (storage.HasFingerprint())
            printFingerprint(storage.Load());
        else
            printFingerprint(generateAndStore(storage));
    }
    catch (...) {}

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
                try { printFingerprint(generateAndStore(storage)); }
                catch (...) {}
            }
            else if (strcmp(line, "LOGS") == 0)
            {
                puf::LogDump();
            }
            else if (strcmp(line, "DEL") == 0)
            {
                try { storage.Delete(); }
                catch (...) {}
            }
            else if (strcmp(line, "RAW") == 0)
            {
                printRawCounts();
            }
            pos = 0;
        }
        else if (pos < sizeof(line) - 1)
        {
            line[pos++] = static_cast<char>(c);
        }
    }
}
