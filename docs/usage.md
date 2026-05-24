# Использование (C++ API)

## Базовая генерация отпечатка

```cpp
#include <esp32_puf_factory.hpp>

puf::Esp32PufFactory factory;
const auto generator = factory.CreateSramPuf(256);
const puf::Fingerprint fp = generator->Generate();
// fp — это std::vector<uint8_t> длиной 32 байта (256 бит)
```

## С пост-обработкой

```cpp
#include <esp32_puf_factory.hpp>
#include <von_neumann_debias.hpp>

puf::Esp32PufFactory factory;
auto raw = factory.CreateSramPuf(512);

// Устранение смещения дебиасингом фон Неймана
auto debiased = std::make_unique<puf::VonNeumannDebias>(std::move(raw), 256);

const puf::Fingerprint fp = debiased->Generate();
```

## Хранение и аутентификация

```cpp
#include <nvs_fingerprint_storage.hpp>
#include <hamming_authenticator.hpp>

puf::NvsFingerprintStorage storage;

// Регистрация (однократно при производстве)
if (!storage.HasFingerprint()) {
    storage.Store(fp);
}

// Аутентификация (порог внутриустройственного расстояния Хэмминга 10%)
puf::HammingAuthenticator auth(storage.Load(), 10.0);
const bool ok = auth.Authenticate(fp);
```

## Принципы генерации отпечатка

**SRAM PUF:**
1. `Esp32PufFactory` размещает `s_sram_puf_buf[64]` в секции `.noinit`.
2. `SramPuf::Generate()` копирует первые `ceil(bits/8)` байт из этого буфера.
3. Источник энтропии — физические разбросы ячеек SRAM при подаче питания.

> Энтропия захватывается **один раз при cold-boot**. После «горячих» ребутов
> (RTS-reset, `idf.py flash` без power-cycle) содержимое `.noinit` сохраняется
> и `Generate()` вернёт тот же отпечаток, что после последнего power-on.
> Для повторного независимого замера нужен физический power-cycle.

## Метрики (`puf_metrics`)

| Метрика | Идеал | Описание |
|---|---|---|
| `IntraHD` | 0.0 | Среднее расстояние Хэмминга между повторными измерениями одного устройства (цель: < 5%) |
| `InterHD` | 0.5 | Среднее расстояние Хэмминга между отпечатками разных устройств (максимальное разделение) |
| `Uniformity` | 0.5 | Доля единиц (баланс битов) |

```cpp
#include <puf_metrics.hpp>

const double intra = puf::metrics::IntraHD({fp1, fp2, fp3});
const double inter = puf::metrics::InterHD({fpDevice1, fpDevice2});
const double uni   = puf::metrics::Uniformity(fp);
```
