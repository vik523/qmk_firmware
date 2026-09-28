SRC += bullfinch.c

# Экономия флеша: без этого прошивка не влезает в ATmega32U4
LTO_ENABLE   = yes
MAGIC_ENABLE = no

# Необязательно: время суток с компьютера (скрипт bullfinch_time.py)
# RAW_ENABLE = yes
