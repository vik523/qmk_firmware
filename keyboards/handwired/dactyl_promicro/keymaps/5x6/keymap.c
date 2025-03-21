#include QMK_KEYBOARD_H
//#include "luna.c"

#define TAPPING_TERM 200
#define IGNORE_MOD_TAP_INTERRUPT
#define OLED_ENABLE_TIMEOUT

#define LAYER_LOCK_IDLE_TIMEOUT 60000 // TUrn off after 60 seconds.

#define _QWERTY 0
#define _LOWER 1
#define _RAISE 2

#define RAISE MO(_RAISE)
#define LOWER MO(_LOWER)

#define SFTLLCK LSFT_T(KC_0)

char last_key[10] = "None";
//char wpm_str[4];
//uint8_t current_wpm = 0;
int oled_timer = 0;

enum custom_keycodes {
    DRAG_SCROLL = RAISE,
};

bool set_scrolling = false;

report_mouse_t pointing_device_task_user(report_mouse_t mouse_report) {
    if (set_scrolling) {
        mouse_report.h = mouse_report.x;
        mouse_report.v = mouse_report.y;
        mouse_report.x = 0;
        mouse_report.y = 0;
    }
    return mouse_report;
}

oled_rotation_t oled_init_user(oled_rotation_t rotation) {
    return OLED_ROTATION_90;
}
/*
static void render_wpm(void) {
    oled_write(" WPM\n", false);
    wpm_str[3] = '\0';
    wpm_str[2] = '0' + current_wpm % 10;
    wpm_str[1] = '0' + (current_wpm /=10) % 10;
    wpm_str[0] = '0' + current_wpm / 10;
    oled_write(" ", false);
    oled_write(wpm_str, false);
}
*/

bool oled_task_user(void) {
    oled_write_P(PSTR("\n"), false);
    switch (get_highest_layer(layer_state)) {
        case _QWERTY:
            oled_write_P(PSTR("QWR\n"), false);
            break;
        case _LOWER:
            oled_write_P(PSTR("LWR\n"), false);
            break;
        case _RAISE:
            oled_write_P(PSTR("RSE\n"), false);
            break;
        default:
            // Or use the write_ln shortcut over adding '\n' to the end of your string
            oled_write_P(PSTR("UND\n"), false);
    }
    // Host Keyboard LED Status
    led_t led_state = host_keyboard_led_state();
    oled_write_ln_P(led_state.num_lock ? PSTR("NUM \n") : PSTR("    \n"), false);
    oled_write_ln_P(led_state.caps_lock ? PSTR("CAP \n") : PSTR("    \n"), false);
    oled_write_ln_P(led_state.scroll_lock ? PSTR("SCR \n") : PSTR("    \n"), false);
    oled_write(last_key, false);
    oled_write("\n", false);
    //render_wpm();
    //int x, y;
    //x = 64;
    //y = 0;
    //render_luna(x, y);
    return false;
    /*
    if (timer_elapsed32(oled_timer) > 60000) {
        oled_off();
        return false;
    }
    else {
       if (!is_oled_on()) 
            oled_on();
        else
            oled_render_idle();
        return false;
    }
    */
}

const uint16_t PROGMEM keymaps[][MATRIX_ROWS][MATRIX_COLS] = {
    [_QWERTY] = LAYOUT_5x6(
        KC_ESC , KC_1  , KC_2  , KC_3  , KC_4  , KC_5  ,                         KC_6  , KC_7  , KC_8  , KC_9  , KC_0  ,KC_BSPC,
        KC_TAB , KC_Q  , KC_W  , KC_E  , KC_R  , KC_T  ,                         KC_Y  , KC_U  , KC_I  , KC_O  , KC_P  ,KC_MINS,
        KC_LSFT, KC_A  , KC_S  , KC_D  , KC_F  , KC_G  ,                         KC_H  , KC_J  , KC_K  , KC_L  ,KC_SCLN,KC_QUOT,
        KC_LCTL, KC_Z  , KC_X  , KC_C  , KC_V  , KC_B  ,                         KC_N  , KC_M  ,KC_COMM,KC_DOT ,KC_SLSH,KC_BSLS,
                         KC_LBRC,KC_RBRC,                                                       KC_PLUS, KC_EQL,
                                         RAISE,KC_SPC,                          KC_BSPC, LOWER,
                                         KC_LALT,KC_HOME,                       _______, KC_ENT,
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
          KC_CAPS,_______,KC_UP  ,_______,KC_LBRC,KC_RBRC,                        KC_BTN1,KC_BTN2,KC_NUM ,KC_INS ,KC_SCRL,KC_MUTE,
          KC_LSFT,KC_LEFT,KC_DOWN,KC_RGHT,_______,KC_LPRN,                        KC_LEFT,KC_DOWN,KC_UP  ,KC_RGHT,_______,KC_VOLU,
          _______,_______,_______,_______,_______,_______,                        KC_RPRN,KC_MPRV,KC_MPLY,KC_MNXT,_______,KC_VOLD,
                                                  _______,_______,            _______, KC_EQL,
                                                  _______,KC_TRNS,            _______,_______,
                                                  _______,_______,            _______, KC_DEL,
                                                  _______,_______,            _______,_______
    )
};

void update_last_key(uint16_t keycode) {
    switch (keycode) {
        case KC_A ... KC_Z:
            snprintf(last_key, sizeof(last_key), "%u", keycode);
            break;
        case KC_1 ... KC_0:
            snprintf(last_key, sizeof(last_key), "%u", keycode);
            break;
        case KC_ENT:
            snprintf(last_key, sizeof(last_key), "Enter");
            break;
        case KC_SPC:
            snprintf(last_key, sizeof(last_key), "Space");
            break;
        case KC_ESC:
            snprintf(last_key, sizeof(last_key), "Esc");
            break;
        case KC_PSCR:
            snprintf(last_key, sizeof(last_key), "PrtSc");
            break;
        case KC_DEL:
            snprintf(last_key, sizeof(last_key), "Del");
            break;
        case KC_END:
            snprintf(last_key, sizeof(last_key), "End");
            break;
        case KC_BSPC:
            snprintf(last_key, sizeof(last_key), "BSPC");
            break;
        case KC_TAB:
            snprintf(last_key, sizeof(last_key), "Tab");
            break;
        default:
            snprintf(last_key, sizeof(last_key), "Other");
            break;
    }
}

bool process_record_user(uint16_t keycode, keyrecord_t *record) {
    if (record->event.pressed) {
        oled_timer = timer_read32();
        //current_wpm = get_current_wpm();
        //set_keylog(keycode, record);
        update_last_key(keycode);
    }
    if (keycode == DRAG_SCROLL && record->event.pressed) {
        set_scrolling = !set_scrolling;
    }
    switch (keycode) {
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