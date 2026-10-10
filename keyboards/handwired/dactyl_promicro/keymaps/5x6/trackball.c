// Трекбол: скролл с блокировкой оси, точный режим, CPI с сохранением в EEPROM,
// автоматический слой мыши.
#include "trackball.h"
#include <stdlib.h>

// ---- Настройки (любую можно переопределить в config.h) ------------------

// Чувствительность при первом включении и после сброса EEPROM
#ifndef TB_CPI_DEFAULT
#    ifdef PMW33XX_CPI
#        define TB_CPI_DEFAULT PMW33XX_CPI
#    else
#        define TB_CPI_DEFAULT 800
#    endif
#endif
#ifndef TB_CPI_STEP
#    define TB_CPI_STEP 200 // шаг клавиш TB_CPIU / TB_CPID
#endif
#ifndef TB_CPI_MIN
#    define TB_CPI_MIN 200
#endif
#ifndef TB_CPI_MAX
#    define TB_CPI_MAX 4000
#endif

// Точный режим: курсор медленнее в N раз
#ifndef TB_SNIPE_DIV
#    define TB_SNIPE_DIV 4
#endif

// Скролл: одна строка на N единиц движения шара (больше — медленнее)
#ifndef TB_SCROLL_DIV
#    define TB_SCROLL_DIV 12
#endif
// Через сколько мс покоя шара можно снова выбрать ось скролла
#ifndef TB_AXIS_RESET_MS
#    define TB_AXIS_RESET_MS 300
#endif
// Направление скролла: раскомментируйте в config.h, если крутит не туда
//   #define TB_SCROLL_INVERT_V
//   #define TB_SCROLL_INVERT_H
// Свободный скролл по диагонали вместо блокировки оси:
//   #define TB_SCROLL_NO_AXIS_LOCK

// ---- Состояние ----------------------------------------------------------

enum { AXIS_NONE, AXIS_V, AXIS_H };

static bool     scroll_hold, scroll_lock, snipe;
static int16_t  acc_x, acc_y;
static uint8_t  axis;
static uint16_t scroll_timer;

static void reset_motion(void) {
    acc_x = acc_y = 0;
    axis          = AXIS_NONE;
}

static void cpi_step(int16_t delta) {
    int16_t cpi = (int16_t)pointing_device_get_cpi() + delta;
    if (cpi < TB_CPI_MIN) cpi = TB_CPI_MIN;
    if (cpi > TB_CPI_MAX) cpi = TB_CPI_MAX;
    pointing_device_set_cpi(cpi);
    eeconfig_update_user(cpi); // переживёт отключение кабеля
}

// ---- Хуки QMK -----------------------------------------------------------

void pointing_device_init_user(void) {
    uint32_t cpi = eeconfig_read_user();
    if (cpi < TB_CPI_MIN || cpi > TB_CPI_MAX) cpi = TB_CPI_DEFAULT;
    pointing_device_set_cpi(cpi);
#ifdef POINTING_DEVICE_AUTO_MOUSE_ENABLE
    set_auto_mouse_layer(TB_MOUSE_LAYER);
    set_auto_mouse_enable(true);
#endif
}

report_mouse_t pointing_device_task_user(report_mouse_t r) {
    if (scroll_hold || scroll_lock) {
        if (r.x == 0 && r.y == 0) {
            // шар остановился — через паузу ось можно выбрать заново
            if (axis != AXIS_NONE && timer_elapsed(scroll_timer) > TB_AXIS_RESET_MS) reset_motion();
            return r;
        }
        scroll_timer = timer_read();
        acc_x += r.x;
        acc_y += r.y;
        r.x = r.y = 0;
#ifndef TB_SCROLL_NO_AXIS_LOCK
        if (axis == AXIS_NONE) {
            if (abs(acc_x) + abs(acc_y) < TB_SCROLL_DIV) return r;
            axis = abs(acc_y) >= abs(acc_x) ? AXIS_V : AXIS_H;
        }
        if (axis == AXIS_V) acc_x = 0;
        else                acc_y = 0;
#endif
        int16_t h = acc_x / TB_SCROLL_DIV, v = acc_y / TB_SCROLL_DIV;
        acc_x -= h * TB_SCROLL_DIV;
        acc_y -= v * TB_SCROLL_DIV;
#ifdef TB_SCROLL_INVERT_H
        h = -h;
#endif
#ifdef TB_SCROLL_INVERT_V
        v = -v;
#endif
        r.h = h;
        r.v = v;
        return r;
    }

    if (snipe) {
        acc_x += r.x;
        acc_y += r.y;
        r.x = acc_x / TB_SNIPE_DIV;
        r.y = acc_y / TB_SNIPE_DIV;
        acc_x -= r.x * TB_SNIPE_DIV;
        acc_y -= r.y * TB_SNIPE_DIV;
    }
    return r;
}

void trackball_scroll(bool on) {
    scroll_hold = on;
    reset_motion();
}

#ifdef POINTING_DEVICE_AUTO_MOUSE_ENABLE
#    ifndef AUTO_MOUSE_THRESHOLD
#        define AUTO_MOUSE_THRESHOLD 10
#    endif
// Когда шар включает слой мыши. Отличие от стандартного: прокрутка слой не
// включает, иначе при RAISE + шар слой мыши всплывал бы поверх RAISE и
// закрывал стрелки.
bool auto_mouse_activation(report_mouse_t r) {
    static int16_t tx, ty;
    if (scroll_hold || scroll_lock) {
        tx = ty = 0;
        return false;
    }
    tx += r.x;
    ty += r.y;
    if (abs(tx) > AUTO_MOUSE_THRESHOLD || abs(ty) > AUTO_MOUSE_THRESHOLD || r.buttons) {
        tx = ty = 0;
        return true;
    }
    return false;
}

// Свои клавиши трекбола не должны выключать слой мыши
bool is_mouse_record_user(uint16_t keycode, keyrecord_t *record) {
    switch (keycode) {
        case TB_SCRL:
        case TB_SCLK:
        case TB_SNIP:
            return true;
    }
    return false;
}
#endif

bool trackball_process_record(uint16_t keycode, keyrecord_t *record) {
    bool pressed = record->event.pressed;
    switch (keycode) {
        case TB_SCRL:
            scroll_hold = pressed;
            reset_motion();
            return false;
        case TB_SCLK:
            if (pressed) {
                scroll_lock = !scroll_lock;
                reset_motion();
            }
            return false;
        case TB_SNIP:
            snipe = pressed;
            reset_motion();
            return false;
        case TB_CPIU:
            if (pressed) cpi_step(TB_CPI_STEP);
            return false;
        case TB_CPID:
            if (pressed) cpi_step(-TB_CPI_STEP);
            return false;
        case TB_AUTO:
#ifdef POINTING_DEVICE_AUTO_MOUSE_ENABLE
            if (pressed) {
                if (get_auto_mouse_enable()) {
                    auto_mouse_layer_off(); // сначала убрать слой, иначе он залипнет
                    set_auto_mouse_enable(false);
                } else {
                    set_auto_mouse_enable(true);
                }
            }
#endif
            return false;
    }
    return true;
}
