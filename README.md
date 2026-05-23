# puf_generator

Hardware device identifier based on PUF for ESP32.
Supports two entropy sources:

- **RO PUF** - pairwise comparison of software IRAM oscillator counters.
- **SRAM PUF** - power-on state of uninitialized SRAM.

Both generate a unique device fingerprint via a shared `IPufGenerator` API.
Select the type at build time: `make flash PUF_TYPE=ro|sram`.

---

- [Сценарий использования](docs/quickstart.md)
- [Архитектура](docs/architecture.md)
- [Требования и сборка](docs/build.md)
- [Использование (C++ API)](docs/usage.md)
- [Сервер верификации](docs/server.md)
