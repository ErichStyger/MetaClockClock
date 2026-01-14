/**
 * @file lv_carousel.h
 *
 */

/**
 * Copyright 2024 NXP
 *
 * SPDX-License-Identifier: MIT
 */

#ifndef LV_CAROUSEL_H
#define LV_CAROUSEL_H

#ifdef __cplusplus
extern "C" {
#endif

/*********************
 *      INCLUDES
 *********************/
#include "../../lv_conf_internal.h"

#if LV_USE_CAROUSEL != 0

#include "../../core/lv_obj.h"

/*********************
 *      DEFINES
 *********************/

/**********************
 *      TYPEDEFS
 **********************/

LV_ATTRIBUTE_EXTERN_DATA extern const lv_obj_class_t lv_carousel_class;
LV_ATTRIBUTE_EXTERN_DATA extern const lv_obj_class_t lv_carousel_element_class;

/**********************
 * GLOBAL PROTOTYPES
 **********************/

/**
 * Create a carousel object
 * @param parent    pointer to an object, it will be the parent of the new carousel
 * @return          pointer to the created carousel
 */
lv_obj_t * lv_carousel_create(lv_obj_t * parent);

/**
 * Add a new element to the carousel
 * @param obj       pointer to a carousel object
 * @param id        identifier for the new element
 * @return          pointer to the created element object
 */
lv_obj_t * lv_carousel_add_element(lv_obj_t * obj, uint8_t id);

/*=====================
 * Setter functions
 *====================*/

/**
 * Set the width of the carousel elements
 * @param obj       pointer to a carousel object
 * @param w         the new width of the elements
 */
void lv_carousel_set_element_width(lv_obj_t * obj, int32_t w);

/**
 * Set an element for the carousel
 * @param obj           pointer to a carousel object
 * @param element_obj   pointer to the element object to set
 * @param anim_en       LV_ANIM_ON: set the element with animation; LV_ANIM_OFF: set the element without animation
 */
void lv_obj_set_element(lv_obj_t * obj, lv_obj_t * element_obj, lv_anim_enable_t anim_en);

/**
 * Set the element ID for a carousel object
 * @param obj       pointer to a carousel object
 * @param id        the new ID to set for the element
 * @param anim_en   LV_ANIM_ON: set the element ID with animation; LV_ANIM_OFF: set the element ID without animation
 */
void lv_obj_set_element_id(lv_obj_t * obj, uint32_t id, lv_anim_enable_t anim_en);

/*=====================
 * Getter functions
 *====================*/

/**
 * Get the currently active element of the carousel
 * @param obj       pointer to a carousel object
 * @return          pointer to the active element object
 */
lv_obj_t * lv_carousel_get_element_active(lv_obj_t * obj);

/*=====================
 * Other functions
 *====================*/

/**********************
 *      MACROS
 **********************/

#endif /*LV_USE_CAROUSEL*/

#ifdef __cplusplus
} /*extern "C"*/
#endif

#endif /*LV_CAROUSEL_H*/
