/*
 * Copyright 2020, 2024 NXP
 *
 * SPDX-License-Identifier: BSD-3-Clause
 */

#include "lvgl.h"
#include "gui_guider.h"

extern void setup_scr_screen(lv_ui *ui);

void setup_ui(lv_ui *ui)
{
    setup_scr_screen(ui);
    lv_scr_load(ui->screen);
}
