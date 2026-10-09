// Снегирь на ветке рябины — сцена для OLED 128x32 (вертикально, 32x128)
#pragma once
#include QMK_KEYBOARD_H

enum bullfinch_sky {
    BF_SKY_NIGHT = 0,
    BF_SKY_DAWN,
    BF_SKY_DAY,
    BF_SKY_DUSK,
    BF_SKY_COUNT
};

// Вызывать из process_record_user (запоминает последнюю клавишу, «вздрагивание»)
void bullfinch_process_record(uint16_t keycode, keyrecord_t *record);

// Рисует сцену. Вызывать из oled_task_user и вернуть её результат.
bool bullfinch_render(void);

// Время суток
void bullfinch_set_sky(uint8_t sky);  // вручную
void bullfinch_next_sky(void);        // по кругу: ночь → рассвет → день → закат
void bullfinch_set_hour(uint8_t hour); // по часу 0..23 (например, из Raw HID)

// Передача последней клавиши на половину без USB (если экран стоит на ней)
void bullfinch_init(void);          // вызвать из keyboard_post_init_user
void bullfinch_housekeeping(void);  // вызвать из housekeeping_task_user
