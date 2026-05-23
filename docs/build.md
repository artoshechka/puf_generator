# Требования и сборка

## Требования

- Python 3.9+ и `pip install pyserial`
- Docker и Docker Compose
- [Conan](https://conan.io/) 2.x + CMake 3.16+ (только для хост-тестов)
- [Doxygen](https://www.doxygen.nl/) 1.9+ (опционально, для документации)

ESP-IDF устанавливается автоматически скриптом при первом запуске `make flash`.

---

## Сборка и запуск

Все команды унифицированы через `Makefile`. Конфигурация — через `.env`:

```bash
cp .env.example .env   # настроить пути и токены под своё окружение
```

| Команда | Действие |
|---|---|
| `make firmware` | Собрать прошивку ESP32 (RO PUF) |
| `make firmware PUF_TYPE=sram` | Собрать прошивку со SRAM PUF |
| `make flash` | Прошить плату и открыть монитор (RO PUF) |
| `make flash PUF_TYPE=sram` | Прошить плату со SRAM PUF |
| `make monitor` | Открыть монитор без перепрошивки |
| `make menuconfig` | Открыть меню конфигурации прошивки |
| `make server` | Собрать Go-бинарь локально |
| `make server-run` | Запустить сервер локально (нужен `DATABASE_URL` в `.env`) |
| `make docker-up` | Запустить сервер + PostgreSQL в Docker |
| `make docker-down` | Остановить контейнеры |
| `make docker-clean` | Остановить контейнеры и удалить БД |
| `make docker-logs` | Следить за логами сервера |
| `make test` | Запустить хост-тесты через Conan + CMake |
| `make puf` | Прочитать PUF-отпечаток с платы |
| `make raw-osc` | Снять сырые счётчики осцилляторов и оценить стабильность |
| `make board-logs` | Забрать логи из буфера платы |

### Конфигурация прошивки (menuconfig)

```bash
idf.py menuconfig   # PUF Generator → длина отпечатка, окно, число раундов
```

| Параметр | По умолчанию | Описание |
|---|---|---|
| `PUF_TYPE` | `ro` | Источник энтропии: `ro` (Ring Oscillator) или `sram` |
| `PUF_FINGERPRINT_BITS` | 256 | Длина отпечатка в битах (64–496) |
| `PUF_WINDOW_CYCLES` | 200 000 | Окно измерения в тактах CPU (только для RO PUF) |
| `PUF_MAJORITY_ROUNDS` | 3 | Число раундов голосования (нечётное) |

### Документация (Doxygen)

```bash
doxygen Doxyfile
# HTML → docs/html/index.html
```
