/**
 * @file lv_carousel_private.h
 *
 */

/**
 * Copyright 2024 NXP
 *
 * SPDX-License-Identifier: MIT
 */

#ifndef LV_CAROUSEL_PRIVATE_H
#define LV_CAROUSEL_PRIVATE_H

#ifdef __cplusplus
extern "C" {
#endif

/*********************
 *      INCLUDES
 *********************/

#include "lv_carousel.h"

#if LV_USE_CAROUSEL != 0
#include "../../core/lv_obj_private.h"

/*********************
 *      DEFINES
 *********************/

/**********************
 *      TYPEDEFS
 **********************/

struct lv_carousel_t {
    lv_obj_t obj;
    lv_obj_t * element_act;
    lv_obj_t * sentinel;
    int32_t element_width;
    int32_t start_pos;
};

struct lv_carousel_element_t {
    lv_obj_t obj;
    lv_dir_t dir;
};


/**********************
 * GLOBAL PROTOTYPES
 **********************/

/**********************
 *      MACROS
 **********************/

#endif /* LV_USE_CAROUSEL != 0 */

#ifdef __cplusplus
} /*extern "C"*/
#endif

#endif /*LV_CAROUSEL_PRIVATE_H*/
