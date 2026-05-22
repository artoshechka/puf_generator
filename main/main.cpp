/// @file main.cpp
/// @brief Точка входа: генерация PUF-отпечатка и командный интерфейс по UART.
///
/// Поддерживаемые команды (отправить строку + '\n' в монитор):
///   PUF   — сгенерировать и вывести новый отпечаток (кешируется в NVS)
///   LOGS  — дамп накопленных логов и очистка буфера
///   DEL   — удалить кешированный отпечаток из NVS

#include <sdkconfig.h>
#include <cstdio>
#include <cstring>

#include <freertos/FreeRTOS.h>
#include <freertos/task.h>
#include <nvs_flash.h>

#include <esp32_puf_factory.hpp>
#include <majority_voter.hpp>
#include <nvs_fingerprint_storage.hpp>
#include <puf_log.hpp>

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
            pos = 0;
        }
        else if (pos < sizeof(line) - 1)
        {
            line[pos++] = static_cast<char>(c);
        }
    }
}
