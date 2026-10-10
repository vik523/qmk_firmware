// Трекбол: скролл с блокировкой оси, точный режим, CPI с сохранением,
// автоматический слой мыши. Подключение — см. README.md рядом.
#pragma once

#include QMK_KEYBOARD_H

// Номер слоя мыши (должен совпадать с _MOUSE в keymap.c)
#ifndef TB_MOUSE_LAYER
#    define TB_MOUSE_LAYER 5
#endif

// Коды клавиш трекбола. Начинаются с отступом от SAFE_RANGE,
// чтобы не пересечься с SKY_NEXT и другими своими кодами.
#ifndef TB_KEYCODE_BASE
#    define TB_KEYCODE_BASE (SAFE_RANGE + 16)
#endif

enum trackball_keycodes {
    TB_SCRL = TB_KEYCODE_BASE, // удерживать — шар крутит страницу
    TB_SCLK,                   // нажать — скролл включён/выключен (защёлка)
    TB_SNIP,                   // удерживать — медленный точный курсор
    TB_CPIU,                   // чувствительность выше (сохраняется)
    TB_CPID,                   // чувствительность ниже (сохраняется)
    TB_AUTO,                   // автослой мыши вкл/выкл
};

// Вызвать в начале process_record_user:
//   if (!trackball_process_record(keycode, record)) return false;
bool trackball_process_record(uint16_t keycode, keyrecord_t *record);

// Включить/выключить скролл из своего кода (у нас — пока зажат RAISE)
void trackball_scroll(bool on);
