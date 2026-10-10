SRC += bullfinch.c trackball.c

# Экономия флеша: без этого прошивка не влезает в ATmega32U4
LTO_ENABLE   = yes
MAGIC_ENABLE = no
# Кнопки мыши (MS_BTN1…) работают через трекбол и без mousekeys,
# а движение курсора с клавиш не используется: выключение освобождает место под trackball.c
MOUSEKEY_ENABLE = no

# Необязательно: время суток с компьютера (скрипт bullfinch_time.py)
# RAW_ENABLE = yes
