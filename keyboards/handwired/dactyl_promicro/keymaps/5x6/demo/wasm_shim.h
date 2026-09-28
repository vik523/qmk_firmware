// Подмена QMK_KEYBOARD_H для сборки bullfinch.c в WebAssembly (демо в браузере)
#pragma once
#include <stdint.h>
#include <stdbool.h>
#include <stddef.h>
#include "keycodes.h"
#include "modifiers.h"

#define PROGMEM
#define pgm_read_byte(p)  (*(const uint8_t *)(p))
#define pgm_read_word(p)  (*(const uint16_t *)(p))
#define pgm_read_dword(p) (*(const uint32_t *)(p))
#define memcpy_P memcpy
#define ARRAY_SIZE(a) (sizeof(a) / sizeof((a)[0]))
#define MOD_BIT_LSHIFT 0x02
#define MOD_MASK_SHIFT 0x22   // как в QMK: левый и правый Shift
#define QK_MODS_GET_MODS(kc) (((kc) >> 8) & 0x1F)
#define QK_MODS_GET_BASIC_KEYCODE(kc) ((kc) & 0xFF)

size_t strlen(const char *s);
void  *memcpy(void *d, const void *s, size_t n);
void  *memset(void *d, int c, size_t n);
int    abs(int v);

typedef struct { struct { bool pressed; } event; struct { uint8_t count; } tap; } keyrecord_t;
typedef union { uint8_t raw; struct { bool num_lock : 1; bool caps_lock : 1; bool scroll_lock : 1; }; } led_t;
typedef uint32_t layer_state_t;

extern layer_state_t layer_state, default_layer_state;
uint8_t  get_highest_layer(layer_state_t s);
led_t    host_keyboard_led_state(void);
uint8_t  get_mods(void);
uint8_t  get_oneshot_mods(void);
uint16_t timer_read(void);
uint16_t timer_elapsed(uint16_t t);
uint32_t last_input_activity_elapsed(void);
void     oled_write_pixel(uint8_t x, uint8_t y, bool on);
void     oled_clear(void);
bool     is_oled_on(void);
bool     oled_off(void);
