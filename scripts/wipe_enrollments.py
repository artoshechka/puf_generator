#!/usr/bin/env python3
"""Удалить все enrollment-записи на сервере PUF.

Назначение
----------
Канонический VN-debias (b0) инвертирует каждый бит отпечатка по
сравнению с прежней реализацией. Все enrollment'ы, собранные до
этого изменения, недействительны и должны быть пересняты после
перепрошивки устройств. Скрипт чистит таблицу devices через
admin-API, не лазая в БД напрямую.

Использование
-------------
    ADMIN_TOKEN=... python3 scripts/wipe_enrollments.py --dry-run
    ADMIN_TOKEN=... python3 scripts/wipe_enrollments.py
    ADMIN_TOKEN=... python3 scripts/wipe_enrollments.py --server https://puf.example/

Exit-коды: 0 успех, 1 ошибка, 2 cancelled пользователем.
"""

import argparse
import json
import os
import sys
import urllib.error
import urllib.request


def _request(method: str, url: str, token: str, expect: tuple[int, ...]) -> tuple[int, bytes]:
    """Выполняет HTTP-запрос с Bearer-токеном; кидает SystemExit при неожиданном статусе."""
    req = urllib.request.Request(url, method=method, headers={"Authorization": f"Bearer {token}"})
    try:
        with urllib.request.urlopen(req, timeout=10) as resp:
            return resp.status, resp.read()
    except urllib.error.HTTPError as e:
        body = e.read().decode("utf-8", errors="replace")
        if e.code in expect:
            return e.code, body.encode("utf-8")
        sys.exit(f"{method} {url}: HTTP {e.code}: {body.strip()}")
    except urllib.error.URLError as e:
        sys.exit(f"{method} {url}: {e.reason}")


def list_devices(server: str, token: str) -> list[dict]:
    status, body = _request("GET", f"{server}/devices", token, expect=(200,))
    if status != 200:
        sys.exit(f"GET /devices: unexpected status {status}")
    return json.loads(body.decode("utf-8"))


def delete_device(server: str, token: str, device_id: str) -> bool:
    """Возвращает True если запись была удалена, False если уже отсутствовала."""
    status, _ = _request("DELETE", f"{server}/devices/{device_id}", token, expect=(204, 404))
    return status == 204


def main() -> None:
    parser = argparse.ArgumentParser(description=__doc__, formatter_class=argparse.RawDescriptionHelpFormatter)
    parser.add_argument("--server", default=os.environ.get("PUF_SERVER", "http://localhost:8080"),
                        help="Базовый URL сервера (по умолчанию http://localhost:8080)")
    parser.add_argument("--dry-run", action="store_true", help="Только показать список, не удалять")
    parser.add_argument("--yes", action="store_true", help="Не запрашивать подтверждение")
    args = parser.parse_args()

    token = os.environ.get("ADMIN_TOKEN")
    if not token:
        sys.exit("ADMIN_TOKEN env var required (see server/config.go)")

    server = args.server.rstrip("/")
    devices = list_devices(server, token)

    if not devices:
        print("No enrollments on server.")
        return

    print(f"Found {len(devices)} enrollment(s) at {server}:")
    for d in devices:
        print(f"  - {d['id']}  enrolled_at={d.get('enrolled_at', '?')}")

    if args.dry_run:
        print("\nDry-run: nothing deleted.")
        return

    if not args.yes:
        reply = input(f"\nDelete all {len(devices)} enrollment(s)? [y/N] ").strip().lower()
        if reply != "y":
            sys.exit(2)

    deleted = 0
    missing = 0
    for d in devices:
        if delete_device(server, token, d["id"]):
            deleted += 1
            print(f"  deleted {d['id']}")
        else:
            missing += 1
            print(f"  already gone: {d['id']}")

    print(f"\nDone: {deleted} deleted, {missing} already absent.")


if __name__ == "__main__":
    main()
