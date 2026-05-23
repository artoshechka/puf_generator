# puf_generator

Аппаратный идентификатор устройства на основе PUF для ESP32.
Поддерживается два источника энтропии:

- **RO PUF** — попарное сравнение счётчиков программных IRAM-осцилляторов.
- **SRAM PUF** — начальное состояние неинициализированной SRAM после включения питания.

Оба генерируют уникальный отпечаток устройства через общий API `IPufGenerator`.
Тип выбирается на этапе сборки: `make flash PUF_TYPE=ro|sram`.

---

## Установка зависимостей

Ставь только то, что нужно для целей, которые планируешь запускать.

### macOS (Apple Silicon / Intel)

```bash
# Прошивка (make firmware, make flash, make menuconfig)
#   ESP-IDF клонируется автоматически через scripts/flash.py при первом запуске.
#   Путь можно переопределить переменной IDF_PATH в .env (по умолчанию ~/esp/esp-idf).

# Сервер (make server, make docker-up)
brew install go              # нужен 1.25+ под текущий go.mod
brew install --cask docker   # или Colima / OrbStack

# Хост-тесты C++ (make test)
brew install conan
conan profile detect         # один раз создаёт ~/.conan2/profiles/default

# Скрипты общения с платой (make puf, make raw-osc, make board-logs)
python3 -m pip install pyserial

# Опционально: API-документация (doxygen Doxyfile → docs/html/)
brew install doxygen
```

### Linux (Debian/Ubuntu)

```bash
sudo apt install golang-1.25 docker.io docker-compose-plugin \
                 python3-serial cmake doxygen pipx
pipx install conan
conan profile detect
```

ESP-IDF — так же, как на macOS: `scripts/flash.py` клонирует в `$IDF_PATH`
при первом `make flash` (укажи `IDF_PATH` в `.env`, если хочешь свой путь).

### Файл окружения

Серверные таргеты требуют учётных данных. Скопируй и поправь:

```bash
cp .env.example .env
# Установи ADMIN_TOKEN — сервер откажется стартовать с пустым токеном.
# POSTGRES_PASSWORD и PUF_THRESHOLD_PCT имеют разумные значения по умолчанию.
```

---

- [Сценарий использования](docs/quickstart.md)
- [Архитектура](docs/architecture.md)
- [Требования и сборка](docs/build.md)
- [Использование (C++ API)](docs/usage.md)
- [Сервер верификации](docs/server.md)
