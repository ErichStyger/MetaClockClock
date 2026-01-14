/*
 * Copyright 2024 NXP
 *
 * SPDX-License-Identifier: BSD-3-Clause
 */

#include "lvgl.h"
#include <stdio.h>
#include "gui_guider.h"
#include "events_init.h"
#include "widgets_init.h"
#include "custom.h"

void setup_scr_screen(lv_ui *ui)
{
    ui->screen = lv_obj_create(NULL);

    ui->btn = lv_btn_create(ui->screen);
    lv_obj_align(ui->btn, LV_ALIGN_CENTER, 0, 0);

    ui->label = lv_label_create(ui->btn);
    lv_label_set_text(ui->label, "Button");

    lv_obj_update_layout(ui->screen);
}
