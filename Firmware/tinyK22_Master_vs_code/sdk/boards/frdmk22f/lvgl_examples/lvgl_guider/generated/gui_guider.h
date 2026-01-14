/*
 * Copyright 2020, 2024 NXP
 *
 * SPDX-License-Identifier: BSD-3-Clause
 */

#ifndef GUI_GUIDER_H
#define GUI_GUIDER_H

#include "lvgl.h"

typedef struct
{
    lv_obj_t *screen;
    lv_obj_t *btn;
    lv_obj_t *label;
} lv_ui;

extern lv_ui guider_ui;

#ifdef __cplusplus
extern "C" {
#endif

void setup_ui(lv_ui *ui);

#ifdef __cplusplus
}
#endif
#endif
