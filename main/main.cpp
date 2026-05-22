/// @file main.cpp
/// @author Artemenko Anton
/// @brief Entry point — generate PUF fingerprint and print it to stdout

#include <sdkconfig.h>
#include <cstdio>
#include <esp32_puf_factory.hpp>
#include <majority_voter.hpp>

extern "C" void app_main()
{
    puf::Esp32PufFactory factory;
    auto raw = factory.CreateRoPuf(CONFIG_PUF_FINGERPRINT_BITS);
    puf::MajorityVoter generator(std::move(raw), CONFIG_PUF_MAJORITY_ROUNDS);

    const puf::Fingerprint fp = generator.Generate();
    for (const uint8_t byte : fp) printf("%02x", byte);
    printf("\n");
}
