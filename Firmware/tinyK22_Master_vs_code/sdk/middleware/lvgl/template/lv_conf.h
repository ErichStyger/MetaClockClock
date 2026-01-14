/**
 * @file lv_conf.h
 */

#ifndef LV_CONF_H
#define LV_CONF_H

/* clang-format off */

/*
 * COLOR DEPTH
 */

#if defined(DEMO_BUFFER_BYTE_PER_PIXEL)

#if DEMO_BUFFER_BYTE_PER_PIXEL == 2
#define LV_COLOR_DEPTH     16
#elif DEMO_BUFFER_BYTE_PER_PIXEL == 4
#define LV_COLOR_DEPTH     32
#elif DEMO_BUFFER_BYTE_PER_PIXEL == 1
#define LV_COLOR_DEPTH     8 /* Can be 8 or 1. */
#else
#error Unsupport color depth
#endif

#endif /* defined(DEMO_BUFFER_BYTE_PER_PIXEL) */

/*
 * BUFFER ALIGNMENT
 */

#if defined(LVGL_FB_ALIGN)

/*Align the stride of all layers and images to this bytes*/
#define LV_DRAW_BUF_STRIDE_ALIGN                LVGL_FB_ALIGN

/*Align the start address of draw_buf addresses to this bytes*/
#define LV_DRAW_BUF_ALIGN                       LVGL_FB_ALIGN

#endif

/* clang-format on */
#endif /*LV_CONF_H*/
