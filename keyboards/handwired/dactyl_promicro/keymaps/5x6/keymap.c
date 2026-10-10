#include QMK_KEYBOARD_H
#include "bullfinch.h"
#include "trackball.h"
#ifdef RAW_ENABLE
#    include "raw_hid.h"
#endif

#define TAPPING_TERM 200
#define IGNORE_MOD_TAP_INTERRUPT

#define _QWERTY 0
#define _LOWER 1
#define _RAISE 2
#define _NUM 3      // цифровой блок, включается TG(_NUM) из слоя ADJ
#define _ADJ 4      // LOWER + RAISE одновременно
#define _MOUSE 5    // включается сам при движении шара (trackball.c)

#define RAISE MO(_RAISE)
#define LOWER MO(_LOWER)

#define SFTLLCK LSFT_T(KC_0)

// Переключение языка — поставьте сочетание, настроенное в системе:
//   LGUI(KC_SPC)  Win+Space (Windows, GNOME)     LALT(KC_LSFT)  Alt+Shift
//   LCTL(KC_LSFT) Ctrl+Shift                      LCTL(KC_SPC)   Ctrl+Space (macOS)
#define LANG_SW LGUI(KC_SPC)

#define CLOSE_W LALT(KC_F4)   // RAISE + X — закрыть окно

enum bullfinch_keycodes {
    SKY_NEXT = SAFE_RANGE,   // сменить время суток: ночь → рассвет → день → закат
};

#ifdef OLED_ENABLE
oled_rotation_t oled_init_user(oled_rotation_t rotation) {
    return OLED_ROTATION_90;
}

bool oled_task_user(void) {
    return bullfinch_render();
}
#endif

#ifdef RAW_ENABLE
// Компьютер присылает пакет: 'T', час (0..23)
void raw_hid_receive(uint8_t *data, uint8_t length) {
    if (length >= 2 && data[0] == 'T') bullfinch_set_hour(data[1]);
}
#endif

const uint16_t PROGMEM keymaps[][MATRIX_ROWS][MATRIX_COLS] = {
    [_QWERTY] = LAYOUT_5x6(
        KC_ESC , KC_1  , KC_2  , KC_3  , KC_4  , KC_5  ,                         KC_6  , KC_7  , KC_8  , KC_9  , KC_0  ,KC_BSPC,
        KC_TAB , KC_Q  , KC_W  , KC_E  , KC_R  , KC_T  ,                         KC_Y  , KC_U  , KC_I  , KC_O  , KC_P  ,KC_MINS,
        KC_LSFT, KC_A  , KC_S  , KC_D  , KC_F  , KC_G  ,                         KC_H  , KC_J  , KC_K  , KC_L  ,KC_SCLN,KC_QUOT,
        KC_LCTL, KC_Z  , KC_X  , KC_C  , KC_V  , KC_B  ,                         KC_N  , KC_M  ,KC_COMM,KC_DOT ,KC_SLSH,KC_BSLS,
                         KC_LBRC,KC_RBRC,                                                       KC_PLUS, KC_EQL,
                                         RAISE,KC_SPC,                          KC_BSPC, LOWER,
                                         KC_LALT,KC_HOME,                       LANG_SW, KC_ENT,
                                         KC_LGUI,KC_GRV,                        KC_DEL, KC_RALT
    ),

    [_LOWER] = LAYOUT_5x6(
        KC_TILD,KC_EXLM, KC_AT ,KC_HASH,KC_DLR ,KC_PERC,                        KC_CIRC,KC_AMPR,KC_ASTR,KC_LPRN,KC_RPRN,KC_DEL,
        _______,_______,_______,_______,_______,KC_LBRC,                        KC_RBRC, KC_P7 , KC_P8 , KC_P9 ,_______,KC_PLUS,
        _______,KC_HOME,KC_PGUP,KC_PGDN,KC_END ,KC_LPRN,                        KC_RPRN, KC_P4 , KC_P5 , KC_P6 ,KC_MINS,KC_PIPE,
        _______,_______,_______,_______,_______,_______,                        KC_PSCR, KC_P1 , KC_P2 , KC_P3 ,KC_EQL ,KC_UNDS,
                                                _______,KC_PSCR,            _______, KC_P0,
                                                _______,_______,            _______,KC_TRNS,
                                                _______,KC_END ,            _______,_______,
                                                _______,_______,            _______,_______

    ),

    [_RAISE] = LAYOUT_5x6(
          KC_F12 , KC_F1 , KC_F2 , KC_F3 , KC_F4 , KC_F5 ,                        KC_F6  , KC_F7 , KC_F8 , KC_F9 ,KC_F10 ,KC_F11 ,
          KC_CAPS,_______,KC_UP  ,_______,KC_LBRC,KC_RBRC,                        MS_BTN1,MS_BTN2,KC_NUM ,KC_INS ,KC_SCRL,KC_MUTE,
          KC_LSFT,KC_LEFT,KC_DOWN,KC_RGHT,_______,KC_LPRN,                        KC_LEFT,KC_DOWN,KC_UP  ,KC_RGHT,_______,KC_VOLU,
          _______,_______,CLOSE_W,_______,_______,_______,                        KC_RPRN,KC_MPRV,KC_MPLY,KC_MNXT,_______,KC_VOLD,
                                                  _______,_______,            _______, KC_EQL,
                                                  _______,KC_TRNS,            _______,_______,
                                                  _______,_______,            _______, KC_DEL,
                                                  _______,_______,            _______,_______
    ),

    // Цифровой блок (фиксируется): справа нумпад, TG(_NUM) — выход
    [_NUM] = LAYOUT_5x6(
        _______,_______,_______,_______,_______,_______,                        _______,KC_NUM ,KC_PSLS,KC_PAST,KC_PMNS,_______,
        _______,_______,_______,_______,_______,_______,                        _______, KC_P7 , KC_P8 , KC_P9 ,KC_PPLS,_______,
        _______,_______,_______,_______,_______,_______,                        _______, KC_P4 , KC_P5 , KC_P6 ,KC_PPLS,_______,
        _______,_______,_______,_______,_______,_______,                        _______, KC_P1 , KC_P2 , KC_P3 ,KC_PENT,_______,
                                                _______,_______,            KC_P0  ,KC_PDOT,
                                                _______,_______,            _______,_______,
                                                _______,_______,            _______,_______,
                                                _______,_______,            _______,TG(_NUM)
    ),

    // Настройки: небо, цифровой блок, прошивка
    [_ADJ] = LAYOUT_5x6(
        QK_BOOT,_______,_______,_______,_______,_______,                        _______,_______,_______,_______,_______,QK_BOOT,
        _______,_______,_______,_______,_______,_______,                        _______,TG(_NUM),_______,_______,_______,_______,
        _______,_______,_______,_______,_______,SKY_NEXT,                       SKY_NEXT,_______,_______,_______,_______,_______,
        _______,_______,_______,TB_CPID,TB_CPIU,TB_AUTO,                        _______,_______,_______,_______,_______,_______,
                                                _______,_______,            _______,_______,
                                                _______,_______,            _______,_______,
                                                _______,_______,            _______,_______,
                                                _______,_______,            _______,_______
    ),

    // Мышь: включается сам, когда двигается шар, и гаснет через 0,65 с после остановки
    // или от любой обычной клавиши. Кнопки на обеих руках.
    [_MOUSE] = LAYOUT_5x6(
        _______,_______,_______,_______,_______,_______,                        _______,_______,_______,_______,_______,_______,
        _______,_______,_______,_______,_______,_______,                        _______,_______,_______,_______,_______,_______,
        _______,TB_SNIP,MS_BTN2,MS_BTN3,MS_BTN1,TB_SCRL,                        TB_SCRL,MS_BTN1,MS_BTN3,MS_BTN2,TB_SNIP,_______,
        _______,_______,_______,MS_BTN4,MS_BTN5,TB_SCLK,                        TB_SCLK,MS_BTN4,MS_BTN5,_______,_______,_______,
                                                _______,_______,            _______,_______,
                                                _______,_______,            _______,_______,
                                                _______,_______,            _______,_______,
                                                _______,_______,            _______,_______
    )
};

layer_state_t layer_state_set_user(layer_state_t state) {
    return update_tri_layer_state(state, _LOWER, _RAISE, _ADJ);
}

bool process_record_user(uint16_t keycode, keyrecord_t *record) {
#ifdef OLED_ENABLE
    bullfinch_process_record(keycode, record);
#endif
    if (!trackball_process_record(keycode, record)) return false;
    // Скролл трекболом, пока зажат RAISE
    if (keycode == RAISE) trackball_scroll(record->event.pressed);
    switch (keycode) {
        case SKY_NEXT:
            if (record->event.pressed) bullfinch_next_sky();
            return false;
        case SFTLLCK:
            if (record->tap.count) {
                if (record->event.pressed) {
                    layer_lock_invert(get_highest_layer(layer_state));
                }
                return false;
            }
            return true;
    }
    return true;
}

void keyboard_post_init_user(void) {
    bullfinch_init();
}

void housekeeping_task_user(void) {
    bullfinch_housekeeping();
}
