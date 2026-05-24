# Сервер верификации (`server/`)

Go-сервер для регистрации устройств и верификации их PUF-отпечатков по расстоянию Хэмминга.

## Быстрый старт (Docker)

```bash
ADMIN_TOKEN=secret docker compose up --build
```

Сервер доступен на `http://localhost:8080`.  
Данные PostgreSQL сохраняются в Docker volume `pgdata` между перезапусками.

Переменные окружения можно переопределить через `.env` в корне:

```bash
# .env
ADMIN_TOKEN=my-secret-token
PUF_THRESHOLD_PCT=8.0
```

### Как это работает

```
┌─────────────────────────────────────┐
│         docker compose up           │
│                                     │
│  ┌──────────────┐  ┌─────────────┐  │
│  │   postgres   │  │   server    │  │
│  │  (healthck)  │──│  (Go bin)   │  │
│  └──────────────┘  └─────────────┘  │
│         pgdata volume               │
└─────────────────────────────────────┘
```

1. `postgres` стартует первым; сервер ждёт `pg_isready` (healthcheck).
2. `server` собирается двухэтапным Dockerfile: Go-тулчейн только в build-стадии, финальный образ — `scratch` (~5 МБ).
3. При старте сервер выполняет `CREATE TABLE IF NOT EXISTS devices` — миграция накатывается автоматически.
4. `pgdata` volume переживает `docker compose down`; для полной очистки: `docker compose down -v`.

## Ручной запуск (без Docker)

### Требования

- Go 1.25+ (см. `server/go.mod`)
- PostgreSQL 14+

```bash
cd server
DATABASE_URL=postgres://user:pass@localhost/pufdb \
ADMIN_TOKEN=secret \
PUF_THRESHOLD_PCT=10.0 \
go run .
```

| Переменная окружения | По умолчанию | Описание |
|---|---|---|
| `DATABASE_URL` | — (обязательная) | DSN PostgreSQL |
| `LISTEN_ADDR` | `:8080` | Адрес и порт сервера |
| `ADMIN_TOKEN` | — (required) | Bearer-токен для админ-операций |
| `PUF_THRESHOLD_PCT` | `10.0` | Максимальный допустимый intra-HD в процентах, диапазон `[0, 50]` |
| `LOG_LEVEL` | `info` | Уровень логирования: `debug` / `info` / `warn` / `error` |

## REST API

```
# Регистрация устройства (admin)
POST /devices/{id}/enroll
Authorization: Bearer <ADMIN_TOKEN>
{"fingerprint": "a1b2c3..."}
→ {"device_id": "esp32-001", "status": "enrolled"}

# Верификация устройства по PUF-отпечатку
POST /devices/{id}/verify
Authorization: PUF <hex-fingerprint>
→ {"ok": true}

# Список устройств (admin)
GET /devices
Authorization: Bearer <ADMIN_TOKEN>

# Удаление устройства (admin)
DELETE /devices/{id}
Authorization: Bearer <ADMIN_TOKEN>

# Метрики сервера (admin)
GET /metrics
Authorization: Bearer <ADMIN_TOKEN>
→ {"enroll_total": 3, "verify_ok": 12, "verify_fail": 2,
   "verify_not_found": 1, "verify_avg_hamming_pct": 3.8}

# Статус сервера (используется Docker healthcheck)
GET /health
→ {"status": "ok"}
```

## Механизм аутентификации

Устройство передаёт свежий PUF-отпечаток в заголовке `Authorization: PUF <hex>`.
Сервер вычисляет расстояние Хэмминга между переданным и эталонным отпечатком.
Если дробное HD (в процентах) не превышает `PUF_THRESHOLD_PCT` — устройство считается подлинным.
