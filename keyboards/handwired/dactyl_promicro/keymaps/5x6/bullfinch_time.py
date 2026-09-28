#!/usr/bin/env python3
"""Отправляет текущий час в клавиатуру, чтобы снегирь знал время суток.

Нужно: RAW_ENABLE = yes в keymaps/5x6/rules.mk и  pip install hidapi
Запуск:  python bullfinch_time.py   (можно добавить в автозагрузку)
"""
import time
import hid

VID, PID = 0xFEED, 0x3060          # из keyboard.json
USAGE_PAGE, USAGE = 0xFF60, 0x61   # стандартный Raw HID интерфейс QMK


def open_keyboard():
    for d in hid.enumerate(VID, PID):
        if d["usage_page"] == USAGE_PAGE and d["usage"] == USAGE:
            dev = hid.device()
            dev.open_path(d["path"])
            return dev
    return None


def main():
    dev, last_hour = None, None
    while True:
        try:
            if dev is None:
                dev = open_keyboard()
                last_hour = None
            hour = time.localtime().tm_hour
            if dev and hour != last_hour:
                # первый байт — report id (0), затем 32 байта данных
                dev.write(bytes([0, ord("T"), hour]) + bytes(30))
                last_hour = hour
        except OSError:
            dev = None          # клавиатуру отключили — переподключимся
        time.sleep(30)


if __name__ == "__main__":
    main()
