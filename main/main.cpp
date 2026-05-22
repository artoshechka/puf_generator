/// @file main.cpp
/// @brief Точка входа: генерация PUF-отпечатка и командный интерфейс по UART.
///
/// Поддерживаемые команды (отправить строку + '\n' в монитор):
///   PUF   — сгенерировать и вывести новый отпечаток
///   LOGS  — дамп накопленных логов и очистка буфера

#include <sdkconfig.h>
#include <cstdio>
#include <cstring>

#include <esp32_puf_factory.hpp>
#include <majority_voter.hpp>
#include <puf_log.hpp>

static void generateAndPrint()
{
    puf::Esp32PufFactory factory;
    auto raw = factory.CreateRoPuf(CONFIG_PUF_FINGERPRINT_BITS);
    puf::MajorityVoter generator(std::move(raw), CONFIG_PUF_MAJORITY_ROUNDS);

    const puf::Fingerprint fp = generator.Generate();
    for (const uint8_t byte : fp) printf("%02x", byte);
    printf("\n");
    fflush(stdout);
}

extern "C" void app_main()
{
    puf::LogInit();

    // Выводим отпечаток сразу при старте.
    generateAndPrint();

    // Ждём команд по UART.
    char line[32];
    size_t pos = 0;
    while (true)
    {
        const int c = getchar();
        if (c == EOF) continue;

        if (c == '\n' || c == '\r')
        {
            line[pos] = '\0';
            if (strcmp(line, "PUF") == 0)
            {
                generateAndPrint();
            }
            else if (strcmp(line, "LOGS") == 0)
            {
                puf::LogDump();
            }
            pos = 0;
        }
        else if (pos < sizeof(line) - 1)
        {
            line[pos++] = static_cast<char>(c);
        }
    }
}
