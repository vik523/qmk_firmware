/*
Copyright 2012 Jun Wako <wakojun@gmail.com>

This program is free software: you can redistribute it and/or modify
it under the terms of the GNU General Public License as published by
the Free Software Foundation, either version 2 of the License, or
(at your option) any later version.

This program is distributed in the hope that it will be useful,
but WITHOUT ANY WARRANTY; without even the implied warranty of
MERCHANTABILITY or FITNESS FOR A PARTICULAR PURPOSE.  See the
GNU General Public License for more details.

You should have received a copy of the GNU General Public License
along with this program.  If not, see <http://www.gnu.org/licenses/>.
*/

#pragma once
#define LANG_USE_ANSI

// ---- OLED / снегирь ----
// Таймаут стандартного драйвера выключаем: экраном управляет bullfinch.c
// (засыпание через BF_SLEEP_MS, выключение через BF_OFF_MS)
#define OLED_TIMEOUT 0
//#define OLED_BRIGHTNESS 120
//#define BF_SLEEP_MS 30000
//#define BF_OFF_MS   300000

// Если OLED стоит на второй (не подключённой к USB) половине —
// эти строки передают ей слой, локи и активность
#define SPLIT_LAYER_STATE_ENABLE
#define SPLIT_LED_STATE_ENABLE
#define SPLIT_ACTIVITY_ENABLE

// Последняя нажатая клавиша для половины без USB (bullfinch.c)
#define SPLIT_TRANSACTION_IDS_USER BF_SYNC_KEY

// ---- Трекбол (trackball.c) ----
// Слой мыши включается сам при движении шара
#define POINTING_DEVICE_AUTO_MOUSE_ENABLE
#define AUTO_MOUSE_DEFAULT_LAYER 5   // = _MOUSE в keymap.c
#define AUTO_MOUSE_THRESHOLD 15      // нечаянное касание шара слой не включит
//#define AUTO_MOUSE_TIME 650        // сколько мс слой живёт после остановки шара
//#define TB_SCROLL_DIV 12           // больше — медленнее скролл
//#define TB_SNIPE_DIV 4             // во сколько раз медленнее точный режим
//#define TB_SCROLL_INVERT_V         // если скролл идёт не в ту сторону
//#define TB_SCROLL_NO_AXIS_LOCK     // свободный скролл по диагонали
