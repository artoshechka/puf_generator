# Unified build facade for puf_generator.
# All targets delegate to the appropriate toolchain — nothing is hardcoded here.
#
# Quickstart:
#   cp .env.example .env   # edit if needed
#   make flash             # build + flash + open monitor
#   make docker-up         # start verification server

-include .env
export

PYTHON     ?= python3
PORT_ARG    = $(if $(ESP_PORT),--port $(ESP_PORT),)

.PHONY: all firmware flash monitor server docker-up docker-down docker-clean \
        test puf board-logs help

all: firmware server

# ── Firmware ──────────────────────────────────────────────────────────────────

## Build ESP32 firmware without flashing
firmware:
	$(PYTHON) scripts/flash.py --build-only

## Flash firmware to the board and open serial monitor
flash:
	$(PYTHON) scripts/flash.py $(PORT_ARG)

## Open serial monitor without reflashing
monitor:
	$(PYTHON) scripts/flash.py --monitor-only $(PORT_ARG)

# ── Go server ─────────────────────────────────────────────────────────────────

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

# ── Host tests ────────────────────────────────────────────────────────────────

## Run host-side unit tests via Conan + CMake
test:
	conan install . --output-folder=build_host --build=missing -pr=profiles/host
	cmake -B build_host -DCMAKE_TOOLCHAIN_FILE=build_host/conan_toolchain.cmake \
	      -DPUF_BUILD_TESTS=ON
	cmake --build build_host
	ctest --test-dir build_host --output-on-failure

# ── Board utilities ───────────────────────────────────────────────────────────

## Read PUF fingerprint from the connected board (stdout only)
puf:
	$(PYTHON) scripts/read_puf.py $(PORT_ARG)

## Fetch accumulated log buffer from the board
board-logs:
	$(PYTHON) scripts/get_board_logs.py $(PORT_ARG)

# ── Help ──────────────────────────────────────────────────────────────────────

help:
	@echo ""
	@echo "Usage: make <target>"
	@echo ""
	@grep -E '^## ' Makefile | sed 's/^## /  /' | \
	    paste - <(grep -E '^[a-z_-]+:' Makefile | sed 's/:.*//' | grep -v '^all') | \
	    awk '{printf "  %-18s %s\n", $$NF, substr($$0, 1, index($$0, $$NF)-1)}'
	@echo ""
