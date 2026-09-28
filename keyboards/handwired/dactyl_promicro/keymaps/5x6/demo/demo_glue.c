// Обвязка для запуска bullfinch.c в браузере: вместо OLED — буфер 32x128
#include "wasm_shim.h"
#include "bullfinch.h"

#define EXPORT(name) __attribute__((export_name(#name)))

static uint8_t  screen[128 * 32];
static bool     screen_on = true;
static uint16_t now_ms;
static uint32_t idle_ms;
static uint8_t  leds, mods;
layer_state_t   layer_state, default_layer_state = 1;

size_t strlen(const char *s) { size_t n = 0; while (s[n]) n++; return n; }
void  *memcpy(void *d, const void *s, size_t n) { uint8_t *a = d; const uint8_t *b = s; while (n--) *a++ = *b++; return d; }
void  *memset(void *d, int c, size_t n) { uint8_t *a = d; while (n--) *a++ = (uint8_t)c; return d; }
int    abs(int v) { return v < 0 ? -v : v; }

uint8_t get_highest_layer(layer_state_t s) { for (int8_t i = 31; i > 0; i--) if (s & (1UL << i)) return i; return 0; }
led_t   host_keyboard_led_state(void) { led_t l; l.raw = leds; return l; }
uint8_t get_mods(void) { return mods; }
uint8_t get_oneshot_mods(void) { return 0; }
uint16_t timer_read(void) { return now_ms; }
uint16_t timer_elapsed(uint16_t t) { return (uint16_t)(now_ms - t); }
uint32_t last_input_activity_elapsed(void) { return idle_ms; }
void oled_write_pixel(uint8_t x, uint8_t y, bool on) { if (x < 32 && y < 128) screen[y * 32 + x] = on; }
void oled_clear(void) { memset(screen, 0, sizeof(screen)); screen_on = true; }
bool is_oled_on(void) { return screen_on; }
bool oled_off(void) { screen_on = false; return true; }

EXPORT(demo_screen)  uint8_t *demo_screen(void) { return screen; }
EXPORT(demo_is_on)   int demo_is_on(void) { return screen_on; }
EXPORT(demo_sky_next) void demo_sky_next(void) { bullfinch_next_sky(); }
EXPORT(demo_set_hour) void demo_set_hour(int h) { bullfinch_set_hour((uint8_t)h); }

// Один «тик» прошивки: время, простой, слой (номер), локи (бит0 Num, бит1 Caps, бит2 Scroll)
EXPORT(demo_tick) void demo_tick(uint32_t ms, uint32_t idle, int layer, int led_bits) {
    now_ms = (uint16_t)ms; idle_ms = idle; leds = (uint8_t)led_bits;
    layer_state = layer ? (1UL << layer) : 0;
    bullfinch_render();
}

// Нажатие клавиши (код QMK), shift — зажат ли Shift
EXPORT(demo_key) void demo_key(int kc, int shift) {
    keyrecord_t r; r.event.pressed = true; r.tap.count = 1;
    mods = shift ? MOD_BIT_LSHIFT : 0;
    bullfinch_process_record((uint16_t)kc, &r);
    mods = 0;
}
