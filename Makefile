# Унифицированный фасад сборки puf_generator.
# Все цели делегируют работу соответствующему тулчейну — ничего не захардкожено здесь.
#
# Быстрый старт:
#   cp .env.example .env   # отредактировать при необходимости
#   make flash             # сборка + прошивка + открытие монитора
#   make docker-up         # запуск сервера верификации

-include .env
export IDF_PATH IDF_TAG ESP_PORT SDKCONFIG_DEFAULTS PUF_TYPE

PYTHON     ?= python3
PORT_ARG    = $(if $(ESP_PORT),--port $(ESP_PORT),)

# Источник энтропии PUF: ro (по умолчанию) или sram.
# Использование: make flash PUF_TYPE=sram
PUF_TYPE   ?= ro
ifeq ($(PUF_TYPE),sram)
SDKCONFIG_DEFAULTS := sdkconfig.defaults;sdkconfig.sram.defaults
else
SDKCONFIG_DEFAULTS := sdkconfig.defaults
endif
export SDKCONFIG_DEFAULTS

.PHONY: all firmware flash monitor server server-run docker-up docker-down docker-clean \
	docker-logs test puf board-logs raw-osc help

all: firmware server

# ── Прошивка ──────────────────────────────────────────────────────────────────

## Build ESP32 firmware without flashing  [PUF_TYPE=ro|sram]
firmware:
	$(PYTHON) scripts/flash.py --build-only

## Flash firmware to the board and open serial monitor  [PUF_TYPE=ro|sram]
flash:
	$(PYTHON) scripts/flash.py $(PORT_ARG)

## Open serial monitor without reflashing
monitor:
	$(PYTHON) scripts/flash.py --monitor-only $(PORT_ARG)

# ── Go-сервер ─────────────────────────────────────────────────────────────────

## Build the Go verification server binary
server:
	cd server && go build -o puf-server .

## Run the server locally (requires DATABASE_URL in .env)
server-run:
	cd server && go run .

# ── Docker ────────────────────────────────────────────────────────────────────

## Build and start server + PostgreSQL in Docker
docker-up:
	docker compose up --build -d

## Stop containers (keep data volume)
docker-down:
	docker compose down

## Stop containers and wipe the database volume
docker-clean:
	docker compose down -v

## Follow server logs
docker-logs:
	docker compose logs -f server

# ── Хост-тесты ────────────────────────────────────────────────────────────────

## Run host-side unit tests via Conan + CMake
test:
	conan profile detect --name host --exist-ok
	conan install . --output-folder=build_host --build=missing -pr=host
	cmake -B build_host -DCMAKE_TOOLCHAIN_FILE=build_host/conan_toolchain.cmake \
	      -DCMAKE_BUILD_TYPE=Release \
	      -DPUF_BUILD_TESTS=ON
	cmake --build build_host
	ctest --test-dir build_host --output-on-failure

## Open firmware configuration menu
menuconfig:
	$(PYTHON) scripts/flash.py --menuconfig

# ── Утилиты для платы ─────────────────────────────────────────────────────────

## Read PUF fingerprint from the connected board (stdout only)
puf:
	@$(PYTHON) scripts/read_puf.py $(PORT_ARG)

## Dump raw oscillator counts and analyze stability (--iterations N)
raw-osc:
	@$(PYTHON) scripts/analyze_raw.py $(PORT_ARG) $(if $(ITER),--iterations $(ITER),)

## Fetch accumulated log buffer from the board
board-logs:
	@$(PYTHON) scripts/get_board_logs.py $(PORT_ARG)

# ── Справка ───────────────────────────────────────────────────────────────────

help:
	@echo ""
	@echo "Usage: make <target>"
	@echo ""
	@grep -E '^## .+|^[a-z_-]+:' Makefile | \
	    awk '/^## / { desc=substr($$0,4) } /^[a-z_-]+:/ { split($$0,a,":"); if (desc) printf "  %-18s %s\n", a[1], desc; desc="" }'
	@echo ""
