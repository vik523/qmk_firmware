#!/bin/sh
# Пересобирает демо из того же bullfinch.c, что идёт в прошивку.
# Нужны clang и wasm-ld (apt install clang lld / brew install llvm).
set -e
cd "$(dirname "$0")"
QMK=../../../../../../quantum
clang --target=wasm32 -O2 -ffreestanding -nostdlib -fno-builtin -w \
    -DQMK_KEYBOARD_H='"wasm_shim.h"' -I. -I.. -I"$QMK" \
    -Wl,--no-entry -Wl,--export-dynamic -Wl,--allow-undefined \
    -o bullfinch.wasm demo_glue.c ../bullfinch.c
python3 - << 'PY'
import base64, re
wasm = base64.b64encode(open("bullfinch.wasm", "rb").read()).decode()
html = open("bullfinch-demo.html", encoding="utf-8").read()
html = re.sub(r'(<script id="wasm" type="application/octet-stream">)[^<]*(</script>)', r'\g<1>' + wasm + r'\g<2>', html)
open("bullfinch-demo.html", "w", encoding="utf-8").write(html)
print("bullfinch-demo.html обновлён, wasm:", len(wasm) * 3 // 4, "байт")
PY
