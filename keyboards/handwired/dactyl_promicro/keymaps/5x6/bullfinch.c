// Снегирь на ветке рябины — сцена для OLED 128x32 (вертикально, 32x128)
//
//  Слой   = где снегирь: QWR — средняя ветка, LWR — нижняя, RSE — верхняя,
//           NUM — с ягодой в клюве, ADJ — в полёте. При смене слоя он перелетает.
//  Caps   = снежная шапка на голове
//  Num    = ягоды рябины «зажжены»
//  Scroll = фонарик-звёздочка над веткой
//  Последняя клавиша — мелко на сугробе внизу
//  Небо   = ночь / рассвет / день / закат

#include "bullfinch.h"

// ---- Настройки (можно переопределить в config.h) -------------------
#ifndef BF_FRAME_MS
#    define BF_FRAME_MS 100          // период анимации, мс
#endif
#ifndef BF_SLEEP_MS
#    define BF_SLEEP_MS 30000        // через сколько простоя снегирь засыпает
#endif
#ifndef BF_OFF_MS
#    define BF_OFF_MS 300000         // через сколько простоя гаснет экран
#endif
#ifndef BF_LAYER_LOWER
#    define BF_LAYER_LOWER 1
#endif
#ifndef BF_LAYER_RAISE
#    define BF_LAYER_RAISE 2
#endif
#ifndef BF_LAYER_NUM
#    define BF_LAYER_NUM 3
#endif
#ifndef BF_LAYER_ADJ
#    define BF_LAYER_ADJ 4
#endif
#ifndef BF_KEY_SCALE
#    define BF_KEY_SCALE 2           // размер буквы на сугробе: 1 — шрифт 3x5, 2 — 6x10
#endif
#define BF_KEY_Y     (127 - 5 * BF_KEY_SCALE - 1)   // верх надписи
#define BF_DRIFT_TOP (BF_KEY_Y - 4)                  // верх сугроба
#ifndef BF_BERRY_EVERY
#    define BF_BERRY_EVERY 450       // раз в сколько кадров падает ягода (450 × 100 мс = 45 с)
#endif

#include "bf_data.inc"

// ---- Состояние ------------------------------------------------------
static uint8_t  bf_sky = BF_SKY_NIGHT;
static char     bf_key[5];
static uint8_t  bf_hop;
static uint16_t bf_tick;
static int16_t  bf_x, bf_y;       // где сейчас снегирь (для перелёта)
static uint8_t  bf_berry_t;       // кадр падения ягоды, 0 — не падает
static uint8_t  bf_fx[8], bf_fy[8];
static bool     bf_init;

// ---- Рисование ------------------------------------------------------
static void px(int16_t x, int16_t y, bool on) {
    if ((uint16_t)x < 32 && (uint16_t)y < 128) oled_write_pixel(x, y, on);
}

static void line(int16_t x0, int16_t y0, int16_t x1, int16_t y1) {
    int16_t dx = abs(x1 - x0), sx = x0 < x1 ? 1 : -1;
    int16_t dy = -abs(y1 - y0), sy = y0 < y1 ? 1 : -1;
    int16_t err = dx + dy;
    for (;;) {
        px(x0, y0, true);
        if (x0 == x1 && y0 == y1) break;
        int16_t e2 = 2 * err;
        if (e2 >= dy) { err += dy; x0 += sx; }
        if (e2 <= dx) { err += dx; y0 += sy; }
    }
}

static void draw_char(int16_t x, int16_t y, char c, bool on) {
    if (c < 32 || c > 126) return;
    uint16_t g = pgm_read_word(&font3x5[c - 32]);
    for (uint8_t r = 0; r < 5; r++)
        for (uint8_t k = 0; k < 3; k++)
            if (g & (1u << (14 - (r * 3 + k)))) px(x + k, y + r, on);
}

static uint8_t sq(int8_t v) { return (uint8_t)(v * v); }

static void stars(const uint8_t (*list)[2], uint8_t n) {
    for (uint8_t i = 0; i < n; i++)
        if ((uint8_t)(bf_tick + i * 5) % 23) px(list[i][0], list[i][1], true);
}

static void forest(int16_t y0) {
    for (uint8_t x = 0; x < 32; x++) {
        uint8_t h = pgm_read_byte(&forest_h[x]);
        for (int16_t y = y0 - h; y <= y0 + 3; y++) px(x, y, false);
        px(x, y0 - h, true);
    }
}

// ---- Небо -----------------------------------------------------------
static const uint8_t night_stars[7][2] = {{4, 6}, {11, 3}, {7, 20}, {14, 12}, {29, 30}, {3, 38}, {18, 44}};
static const uint8_t dawn_stars[2][2]  = {{27, 5}, {22, 12}};
static const uint8_t dusk_stars[2][2]  = {{5, 4}, {28, 8}};

static void sky_night(void) {
    for (int8_t y = 8; y <= 20; y++)
        for (int8_t x = 16; x <= 28; x++)
            if (sq(x - 22) + sq(y - 14) <= 36 && sq(x - 25) + sq(y - 12) > 30) px(x, y, true);
    stars(night_stars, 7);
}

static void sky_dawn(void) {
    for (int8_t y = 24; y <= 30; y++)
        for (int8_t x = 4; x <= 16; x++)
            if (sq(x - 10) + sq(y - 30) <= 36) px(x, y, true);
    if (bf_tick % 8 < 5) {
        line(10, 19, 10, 21); line(2, 24, 4, 25); line(18, 24, 16, 25);
        line(0, 30, 2, 30);   line(20, 30, 18, 30);
    }
    forest(31);
    for (uint8_t x = 1; x < 32; x += 3) px(x, 27 + (x & 1), true);
    stars(dawn_stars, 2);
}

static void sky_day(void) {
    for (int8_t y = 8; y <= 16; y++)
        for (int8_t x = 18; x <= 26; x++)
            if (sq(x - 22) + sq(y - 12) <= 13) px(x, y, true);
    uint8_t ph = (bf_tick % 10 < 5) ? 0 : 8;
    for (uint8_t k = 0; k < 8; k++) {
        const int8_t *r = sun_rays[ph + k];
        line(22 + (int8_t)pgm_read_byte(&r[0]), 12 + (int8_t)pgm_read_byte(&r[1]),
             22 + (int8_t)pgm_read_byte(&r[2]), 12 + (int8_t)pgm_read_byte(&r[3]));
    }
    int16_t cx = (int16_t)((bf_tick / 12) % 44) - 12;   // облако плывёт
    for (uint8_t y = 0; y < 5; y++) {
        uint16_t on = pgm_read_word(&cloud_on[y]), off = pgm_read_word(&cloud_off[y]);
        for (uint8_t x = 0; x < 13; x++) {
            if (on & (1u << x)) px(cx + x, 26 + y, true);
            else if (off & (1u << x)) px(cx + x, 26 + y, false);
        }
    }
}

static void sky_dusk(void) {
    for (int8_t y = 22; y <= 30; y++)
        for (int8_t x = 8; x <= 24; x++)
            if (sq(x - 16) + sq(y - 30) <= 64 && !(y >= 23 && y % 3 == 0)) px(x, y, true);
    forest(31);
    stars(dusk_stars, 2);
}

// ---- Сцена: ветки, рябина, сугроб, снегирь -------------------------
typedef struct {
    uint8_t br_x0, br_y;          // ветка: от br_x0 до правого края, средняя линия br_y
    uint8_t bird_x, bird_y;
    uint8_t anc_x, anc_y;         // откуда свисает гроздь
    uint8_t n, bx[3], by[3];      // ягоды (левый верхний угол 4x4)
} bf_layout_t;

enum { LY_QWR, LY_LWR, LY_RSE, LY_NUM, LY_ADJ };
static const bf_layout_t PROGMEM layouts[5] = {
    {0, 80, 4, 65, 27, 81, 3, {25, 28, 26}, {84, 85, 88}},   // QWR — средняя ветка
    {0, 104, 4, 89, 27, 105, 2, {25, 28, 0}, {107, 108, 0}}, // LWR — нижняя
    {8, 49, 6, 34, 26, 51, 3, {24, 27, 25}, {54, 55, 58}},   // RSE — верхняя
    {0, 80, 4, 65, 27, 81, 2, {25, 28, 0}, {84, 85, 0}},     // NUM — ягода в клюве
    {0, 80, 7, 38, 27, 81, 3, {25, 28, 26}, {84, 85, 88}},   // ADJ — в полёте
};

static void branch(uint8_t x0, uint8_t ym) {
    for (uint8_t x = x0; x < 32; x++) {
        px(x, ym - 1, true);
        px(x, ym + 1, true);
        px(x, ym, x == x0 && x0 > 0);
        uint8_t h = (uint8_t)(x * 7 + (ym - 1) * 3) % 11;   // снег на ветке
        if (h < 6) px(x, ym - 2, true);
        if (h < 2) px(x, ym - 3, true);
    }
}

static void berry(uint8_t x, uint8_t y, bool lit) {
    for (uint8_t j = 0; j < 4; j++)
        for (uint8_t k = 0; k < 4; k++) {
            bool corner = (j == 0 || j == 3) && (k == 0 || k == 3);
            if (corner) continue;
            bool inner = (j == 1 || j == 2) && (k == 1 || k == 2);
            bool on    = lit ? !(j == 1 && k == 1) : (!inner || (j == 1 && k == 1));
            px(x + k, y + j, on);
        }
}

static void lantern(uint8_t x, uint8_t y) {   // Scroll Lock: звёздочка-фонарик
    px(x, y - 2, true); px(x, y + 2, true); px(x - 2, y, true); px(x + 2, y, true);
    px(x, y - 1, true); px(x, y + 1, true); px(x - 1, y, true); px(x + 1, y, true);
    if (bf_tick & 4) { px(x - 1, y - 1, true); px(x + 1, y + 1, true); px(x + 1, y - 1, true); px(x - 1, y + 1, true); }
}

static void drift(void) {
    // Надпись — на чистом снегу: точечная текстура сугроба вокруг неё не рисуется,
    // иначе мелкие буквы сливаются с точками
    uint8_t len = strlen(bf_key);
    uint8_t w   = len ? len * 4 * BF_KEY_SCALE - BF_KEY_SCALE : 0;
    uint8_t kx  = (32 - w) / 2;
    for (uint8_t x = 0; x < 32; x++) {
        uint8_t tp    = BF_DRIFT_TOP + (int8_t)pgm_read_byte(&drift_h[x]);
        bool    clear = len && x + 2 >= kx && x <= kx + w + 1;
        for (uint8_t y = tp; y < 128; y++)
            px(x, y, y == tp || (!clear && x % 3 == 0 && y % 3 == 0));
    }
    for (uint8_t i = 0; i < len; i++) {
        uint16_t g = pgm_read_word(&font3x5[(uint8_t)bf_key[i] - 32]);
        for (uint8_t r = 0; r < 5; r++)
            for (uint8_t k = 0; k < 3; k++)
                if (g & (1u << (14 - (r * 3 + k))))
                    for (uint8_t dy = 0; dy < BF_KEY_SCALE; dy++)
                        for (uint8_t dx = 0; dx < BF_KEY_SCALE; dx++)
                            px(kx + (i * 4 + k) * BF_KEY_SCALE + dx, BF_KEY_Y + r * BF_KEY_SCALE + dy, true);
    }
}

static void sprite(const uint32_t *rows, const uint32_t *mask, uint8_t h, int16_t x0, int16_t y0) {
    for (uint8_t y = 0; y < h; y++) {
        uint32_t r = pgm_read_dword(&rows[y]), m = pgm_read_dword(&mask[y]);
        for (uint8_t x = 0; x < 21; x++)
            if (m & (1UL << x)) px(x0 + x, y0 + y, r & (1UL << x));   // вне силуэта фон не трогаем
    }
}

static void hat(int16_t x0, int16_t y0) {   // Caps Lock — снежная шапка
    for (uint8_t x = 1; x <= 6; x++) px(x0 + x, y0 - 1, true);
    for (uint8_t x = 2; x <= 5; x++) px(x0 + x, y0 - 2, true);
    px(x0 + 3, y0 - 3, true); px(x0 + 4, y0 - 3, true);
}

static int16_t approach(int16_t cur, int16_t target) {   // плавный подлёт
    int16_t d = target - cur, s = d / 3;
    if (!s) s = (d > 0) - (d < 0);
    return cur + s;
}

// ---- Публичные функции ---------------------------------------------
void bullfinch_set_sky(uint8_t sky) { bf_sky = sky % BF_SKY_COUNT; }
void bullfinch_next_sky(void) { bullfinch_set_sky(bf_sky + 1); }
void bullfinch_set_hour(uint8_t h) {
    bullfinch_set_sky(h >= 5 && h < 8 ? BF_SKY_DAWN : h >= 8 && h < 18 ? BF_SKY_DAY : h >= 18 && h < 21 ? BF_SKY_DUSK : BF_SKY_NIGHT);
}

bool bullfinch_render(void) {
    static uint16_t frame_timer;
    uint32_t idle = last_input_activity_elapsed();

    if (idle > BF_OFF_MS) {                 // долгий простой — гасим экран
        if (is_oled_on()) oled_off();
        return false;
    }
    if (timer_elapsed(frame_timer) < BF_FRAME_MS) return false;
    frame_timer = timer_read();
    bf_tick++;

    if (!bf_init) {
        for (uint8_t i = 0; i < 8; i++) { bf_fx[i] = (i * 11 + 5) % 32; bf_fy[i] = (i * 29) % 112; }
        bf_init = true;
    }

    oled_clear();
    bool  sleeping = idle > BF_SLEEP_MS;
    led_t led      = host_keyboard_led_state();

    // снег
    uint8_t nfl = bf_sky == BF_SKY_DAY ? 4 : 8;
    for (uint8_t i = 0; i < 8; i++) {
        if ((uint8_t)(bf_tick + i) % 2 == 0 && ++bf_fy[i] >= 112) bf_fy[i] = 0;
        if ((uint8_t)(bf_tick + i * 3) % 13 == 0) bf_fx[i] = (bf_fx[i] + ((i & 1) ? 1 : 31)) % 32;
        if (i < nfl) px(bf_fx[i], bf_fy[i], true);
    }

    switch (bf_sky) {
        case BF_SKY_DAWN: sky_dawn(); break;
        case BF_SKY_DAY:  sky_day();  break;
        case BF_SKY_DUSK: sky_dusk(); break;
        default:          sky_night(); break;
    }

    drift();

    layer_state_t ls = layer_state;
#ifdef POINTING_DEVICE_AUTO_MOUSE_ENABLE
    ls = remove_auto_mouse_layer(ls, true);   // слой мыши снегирь не замечает
#endif
    uint8_t layer = get_highest_layer(ls | default_layer_state);
    uint8_t li = layer == BF_LAYER_LOWER ? LY_LWR : layer == BF_LAYER_RAISE ? LY_RSE
               : layer == BF_LAYER_NUM   ? LY_NUM : layer == BF_LAYER_ADJ   ? LY_ADJ : LY_QWR;
    const bf_layout_t *L = &layouts[li];
    uint8_t bry = pgm_read_byte(&L->br_y);
    branch(pgm_read_byte(&L->br_x0), bry);

    uint8_t ax = pgm_read_byte(&L->anc_x), ay = pgm_read_byte(&L->anc_y), n = pgm_read_byte(&L->n);
    for (uint8_t i = 0; i < n; i++) line(ax, ay, pgm_read_byte(&L->bx[i]) + 1, pgm_read_byte(&L->by[i]));
    // падающая ягода: отрывается нижняя ягода грозди, падает в сугроб, полежит и исчезнет
    if (!bf_berry_t && !sleeping && bf_tick % BF_BERRY_EVERY == 0) bf_berry_t = 1;
    uint8_t last = n - 1, lbx = pgm_read_byte(&L->bx[last]), lby = pgm_read_byte(&L->by[last]);
    for (uint8_t i = 0; i < n; i++)
        if (!(bf_berry_t && i == last)) berry(pgm_read_byte(&L->bx[i]), pgm_read_byte(&L->by[i]), led.num_lock);
    if (bf_berry_t) {
        int16_t y = lby + bf_berry_t * 2, x = lbx + ((bf_berry_t >> 2) & 1);
        if (y > BF_DRIFT_TOP - 4) y = BF_DRIFT_TOP - 4;   // лежит на сугробе
        berry(x, y, led.num_lock);
        if (++bf_berry_t > (BF_DRIFT_TOP - 4 - lby) / 2 + 25) bf_berry_t = 0;
    }

    if (led.scroll_lock) {   // фонарик висит под веткой слева
        uint8_t lx = pgm_read_byte(&L->br_x0) + 4;
        line(lx, bry + 2, lx, bry + 4);
        lantern(lx, bry + 7 < BF_DRIFT_TOP - 3 ? bry + 7 : BF_DRIFT_TOP - 3);
    }

    // ---- снегирь ----
    int16_t tx = pgm_read_byte(&L->bird_x), ty = pgm_read_byte(&L->bird_y);
    if (li == LY_ADJ) ty += (bf_tick >> 2) & 1;              // парит, чуть покачиваясь
    if (!bf_x) { bf_x = tx; bf_y = ty; }
    bool moving = bf_x != tx || bf_y != ty;
    if (moving) { bf_x = approach(bf_x, tx); bf_y = approach(bf_y, ty); }

    if (moving || li == LY_ADJ) {                             // полёт: машет крыльями
        bool up = bf_tick & 1;
        sprite(up ? fly_up_rows : fly_dn_rows, up ? fly_up_mask : fly_dn_mask, 13, bf_x, bf_y);
        if (led.caps_lock) hat(bf_x + 1, bf_y + 3);
    } else if (sleeping) {                                    // нахохлился и спит
        sprite(sleep_rows, sleep_mask, 13, bf_x, bf_y + 1);
        if (led.caps_lock) hat(bf_x + 2, bf_y + 1);
        uint8_t z = (bf_tick / 8) % 3;
        draw_char(bf_x + 15 + z, bf_y - 4 - z * 5, 'Z', true);
    } else {                                                  // сидит
        int16_t y = bf_y;
        if (bf_hop) { y--; bf_hop--; }
        sprite(bird_rows, bird_mask, 14, bf_x, y);
        bool drowsy = idle > BF_SLEEP_MS - 4000;              // перед сном глаза слипаются
        if (drowsy ? (bf_tick & 8) : (bf_tick % 40) < 3) px(bf_x + 3, y + 2, false);
        if (li == LY_NUM) berry(bf_x - 3, y + 3, led.num_lock);   // ягода в клюве
        if (led.caps_lock) hat(bf_x, y);
    }
    return false;
}

// ---- Имя клавиши ----------------------------------------------------
typedef struct { uint16_t kc; char name[4]; } bf_name_t;
static const bf_name_t PROGMEM names[] = {
    {KC_ENT, "ENT"}, {KC_ESC, "ESC"}, {KC_BSPC, "BSP"}, {KC_TAB, "TAB"}, {KC_SPC, "SPC"},
    {KC_CAPS, "CAP"}, {KC_DEL, "DEL"}, {KC_INS, "INS"}, {KC_HOME, "HOM"}, {KC_END, "END"},
    {KC_PGUP, "PGU"}, {KC_PGDN, "PGD"}, {KC_PSCR, "PRT"}, {KC_SCRL, "SCR"}, {KC_NUM, "NUM"},
    {KC_UP, "u"}, {KC_DOWN, "d"}, {KC_LEFT, "l"}, {KC_RGHT, "r"},
    {KC_LSFT, "SFT"}, {KC_RSFT, "SFT"}, {KC_LCTL, "CTL"}, {KC_RCTL, "CTL"},
    {KC_LALT, "ALT"}, {KC_RALT, "ALT"}, {KC_LGUI, "GUI"}, {KC_RGUI, "GUI"},
    {KC_PENT, "ENT"}, {KC_PDOT, "."}, {KC_PSLS, "/"}, {KC_PAST, "*"}, {KC_PMNS, "-"}, {KC_PPLS, "+"},
    {KC_MUTE, "MUT"}, {KC_VOLU, "V+"}, {KC_VOLD, "V-"}, {KC_MPRV, "PRV"}, {KC_MPLY, "PLY"}, {KC_MNXT, "NXT"},
    {MS_BTN1, "M1"}, {MS_BTN2, "M2"}, {MS_BTN3, "M3"},
};
// символы для KC_MINS..KC_SLSH (0x2D..0x38) без и с Shift
static const char PROGMEM sym_plain[] = "-=[]\\#;'`,./";
static const char PROGMEM sym_shift[] = "_+{}|~:\"~<>?";
static const char PROGMEM num_shift[] = "!@#$%^&*()";

void bullfinch_process_record(uint16_t kc, keyrecord_t *record) {
    if (!record->event.pressed) return;

    if (IS_QK_MOD_TAP(kc) || IS_QK_LAYER_TAP(kc)) {
        if (record->tap.count == 0) return;     // удержание — не «нажатая клавиша»
        kc &= 0xFF;
    }
    bool shift = (get_mods() | get_oneshot_mods()) & MOD_MASK_SHIFT;
    if (IS_QK_MODS(kc)) {
        if (QK_MODS_GET_MODS(kc) & MOD_LSFT) shift = true;
        kc = QK_MODS_GET_BASIC_KEYCODE(kc);
    }
    if (!IS_BASIC_KEYCODE(kc) && !IS_MODIFIER_KEYCODE(kc) && !IS_CONSUMER_KEYCODE(kc) && !IS_MOUSE_KEYCODE(kc)) return;   // слои, макросы и т.п.

    bf_hop = 2;
    bf_key[1] = 0;
    if (kc >= KC_A && kc <= KC_Z) { bf_key[0] = 'A' + (kc - KC_A); return; }
    if (kc >= KC_1 && kc <= KC_0) {
        uint8_t i = kc - KC_1;
        bf_key[0] = shift ? pgm_read_byte(&num_shift[i]) : (i == 9 ? '0' : '1' + i);
        return;
    }
    if (kc >= KC_MINS && kc <= KC_SLSH) {
        bf_key[0] = pgm_read_byte(&(shift ? sym_shift : sym_plain)[kc - KC_MINS]);
        return;
    }
    if (kc >= KC_P1 && kc <= KC_P0) { bf_key[0] = (kc == KC_P0) ? '0' : '1' + (kc - KC_P1); return; }
    if (kc >= KC_F1 && kc <= KC_F12) {
        uint8_t n = kc - KC_F1 + 1;
        bf_key[0] = 'F';
        if (n < 10) { bf_key[1] = '0' + n; bf_key[2] = 0; }
        else        { bf_key[1] = '1'; bf_key[2] = '0' + n - 10; bf_key[3] = 0; }
        return;
    }
    for (uint8_t i = 0; i < ARRAY_SIZE(names); i++) {
        if (pgm_read_word(&names[i].kc) == kc) {
            memcpy_P(bf_key, names[i].name, 4);
            bf_key[4] = 0;
            return;
        }
    }
    bf_key[0] = '?';
}

// ---- Передача последней клавиши на вторую половину ------------------
// Нажатия обрабатывает только половина с USB; если экран стоит на другой,
// она узнаёт о клавише отсюда.
#include "transactions.h"

static void bf_key_rx(uint8_t in_len, const void *in_data, uint8_t out_len, void *out_data) {
    if (in_len == sizeof(bf_key)) {
        memcpy(bf_key, in_data, sizeof(bf_key));
        bf_key[sizeof(bf_key) - 1] = 0;
        bf_hop = 2;   // снегирь вздрагивает и на второй половине
    }
}

void bullfinch_init(void) {
    transaction_register_rpc(BF_SYNC_KEY, bf_key_rx);
}

void bullfinch_housekeeping(void) {
    // bf_hop == 2 сразу после нажатия: отправляем клавишу один раз
    if (is_keyboard_master() && bf_hop == 2) {
        transaction_rpc_send(BF_SYNC_KEY, sizeof(bf_key), bf_key);
        bf_hop = 1;
    }
}
