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
