/// @file main.cpp
/// @brief Точка входа: генерация PUF-отпечатка и командный интерфейс по UART.
///
/// Поддерживаемые команды (отправить строку + '\n' в монитор):
///   PUF   — сгенерировать и вывести новый отпечаток
///   LOGS  — дамп накопленных логов и очистка буфера

#include <esp_log.h>
#include <freertos/FreeRTOS.h>
#include <freertos/task.h>
#include <sdkconfig.h>

#include <cstdio>
#include <cstring>
#include <esp32_puf_factory.hpp>
#include <exception>
#include <puf_log.hpp>

namespace
{

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
    auto raw = factory.CreateSramPuf(CONFIG_PUF_FINGERPRINT_BITS);
    return raw->Generate();
}

}  // namespace

extern "C" void app_main()
{
    puf::LogInit();

    try
    {
        printFingerprint(generate());
    } catch (const std::exception& e)
    {
        ESP_LOGE("main", "PUF generate failed at boot: %s", e.what());
    }

    char line[32];
    size_t pos = 0;
    while (true)
    {
        const int c = getchar();
        if (c == EOF)
        {
            // 10 мс гарантированно выдают как минимум 1 тик при default TICK_RATE=100Hz,
            // в отличие от pdMS_TO_TICKS(1) который округляется до 0 и не даёт IDLE-задаче
            // достаточно процессорного времени — task_wdt начинает срабатывать.
            vTaskDelay(pdMS_TO_TICKS(10));
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
                } catch (const std::exception& e)
                {
                    ESP_LOGE("main", "PUF generate failed: %s", e.what());
                }
            } else if (strcmp(line, "LOGS") == 0)
            {
                puf::LogDump();
            } else
            {
                // Неизвестная команда — игнорируем.
            }
            pos = 0;
        } else if (pos < sizeof(line) - 1U)
        {
            line[pos] = static_cast<char>(c);
            ++pos;
        } else
        {
            // Строка слишком длинная — игнорируем лишний символ.
        }
    }
}
