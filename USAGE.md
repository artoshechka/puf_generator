# Usage Guide

Step-by-step walkthrough: from powering on the board to verifying a device.

---

## Prerequisites

- ESP32 board connected via USB
- Docker and Docker Compose installed
- Python 3.9+ (for the flash script)

---

## 1. Start the verification server

```bash
ADMIN_TOKEN=secret docker compose up --build
```

Wait until you see:

```
puf-server-1  | {"level":"INFO","msg":"puf-server starting","addr":":8080"}
```

The server is now running at `http://localhost:8080`.

---

## 2. Flash the board and read the fingerprint

First flash the firmware (done once):

```bash
python3 scripts/flash.py --build-only   # build
python3 scripts/flash.py                # build + flash + open monitor
```

The script installs ESP-IDF automatically on first run.

**To capture the fingerprint automatically** use `read_puf.py`. It resets the board, waits for the hex output, and prints only the fingerprint to stdout:

```bash
pip install pyserial          # one-time dependency

python3 scripts/read_puf.py   # auto-detect port
```

Output (stderr shows status, stdout is clean for piping):

```
Opening /dev/cu.usbmodem101 at 115200 baud ...
Waiting for fingerprint (timeout 15s) ...
  skip: ESP-ROM:esp32s3-20210327
  skip: Build:Mar 27 2021
  [1/1] captured 256-bit fingerprint
a3f1c8b2e04d7a91f5630be28c1d4f67a9e2b05c3d8f1a74e6c2901b5d7e8f3
```

Capture into a variable for direct use in curl:

```bash
PUF=$(python3 scripts/read_puf.py)
```

Every power cycle produces a slightly different value — that is expected (intra-device noise). The server tolerates up to 10% bit difference by default.

---

## 3. Enroll the device

Run this once — during manufacturing or first setup. Replace `esp32-001` with any unique ID for your device.

```bash
PUF=$(python3 scripts/read_puf.py)

curl -X POST http://localhost:8080/devices/esp32-001/enroll \
  -H "Authorization: Bearer secret" \
  -H "Content-Type: application/json" \
  -d "{\"fingerprint\": \"$PUF\"}"
```

Expected response:

```json
{"device_id": "esp32-001", "status": "enrolled"}
```

The fingerprint is stored in PostgreSQL as the reference for this device.

---

## 4. Verify the device

Power-cycle the board to get a fresh fingerprint, then verify against the enrolled reference:

```bash
PUF=$(python3 scripts/read_puf.py)

curl -X POST http://localhost:8080/devices/esp32-001/verify \
  -H "Authorization: PUF $PUF"
```

**Verified** — the fingerprint is close enough to the reference:

```json
{"ok": true, "hamming_pct": 3.9, "threshold_pct": 10.0}
```

**Rejected** — fingerprint is too far from the reference (wrong or cloned device):

```json
{"ok": false, "hamming_pct": 47.2, "threshold_pct": 10.0}
```

HTTP `200` on success, `401` on rejection.

---

## 5. Manage devices

```bash
# List all enrolled devices
curl http://localhost:8080/devices \
  -H "Authorization: Bearer secret"
```

```json
[
  {"id": "esp32-001", "enrolled_at": "2026-05-22T10:00:00Z"},
  {"id": "esp32-002", "enrolled_at": "2026-05-22T10:05:00Z"}
]
```

```bash
# Remove a device
curl -X DELETE http://localhost:8080/devices/esp32-001 \
  -H "Authorization: Bearer secret"
```

---

## Adjusting the Hamming threshold

The default tolerance is 10%. To tighten or loosen it:

```bash
PUF_THRESHOLD_PCT=5.0 ADMIN_TOKEN=secret docker compose up
```

Lower values → stricter verification (more false rejections under noise).  
Higher values → more tolerant (risk of accepting a similar but different chip).  
Typical intra-device noise for RO PUF on ESP32 is under 5%.

---

## Stopping the server

```bash
# Stop but keep the database
docker compose down

# Stop and wipe all enrolled devices
docker compose down -v
```
