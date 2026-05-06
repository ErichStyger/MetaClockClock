/*
 * Copyright (c) 2020, Erich Styger
 *
 * SPDX-License-Identifier: BSD-3-Clause
 */

#ifndef NEOSTEPPERRING_H_
#define NEOSTEPPERRING_H_

#include "platform.h"
#include <stdint.h>
#include <stdbool.h>
#include <stddef.h>
#include "NeoStepperRingConfig.h"

typedef void *NEOSR_Handle_t;

typedef struct NEOSR_Config_t {
  bool ledCw;      /* if LEDs are organized cw (clockwise) or ccw (counter-clockwise) */
  int ledLane;     /* LED lane */
  int ledStartPos; /* LED starting position in lane */
  struct {
    bool enabled; /* if LED is on or off */
    uint8_t red, green, blue; /* colors for hand LED */
  } hand;
  struct {
    bool enabled; /* if LED is on or off */
    uint8_t red, green, blue; /* colors for ring LED */
  } ring;
} NEOSR_Config_t;

/*!
 * \brief Fills a NeoStepperRing configuration with default values.
 * \param config Configuration structure to fill or use.
 */
void NEOSR_GetDefaultConfig(NEOSR_Config_t *config);

/*!
 * \brief Initializes a NeoStepperRing device instance.
 * \param config Configuration structure to fill or use.
 * \return Requested handle.
 */
NEOSR_Handle_t NEOSR_InitDevice(NEOSR_Config_t *config);

/* set the LEDs for the hand, including dual-hands */
/*!
 * \brief Updates the NeoStepperRing LEDs for the current stepper position.
 * \param device NeoStepperRing device handle.
 * \param stepperPos Current stepper position.
 */
void NEOSR_Illuminate(NEOSR_Handle_t device, int32_t stepperPos);

/*!
 * \brief Enables or disables the main hand LED for a NeoStepperRing device.
 * \param device NeoStepperRing device handle.
 * \param on True to enable or turn on, false to disable or turn off.
 */
void NEOSR_SetHandLedEnabled(NEOSR_Handle_t device, bool on);
/*!
 * \brief Returns whether the main hand LED is enabled.
 * \param device NeoStepperRing device handle.
 * \return True if the condition or operation succeeds, false otherwise.
 */
bool NEOSR_GetHandLedEnabled(NEOSR_Handle_t device);
/*!
 * \brief Sets the main hand LED color.
 * \param device NeoStepperRing device handle.
 * \param red Red color component.
 * \param green Green color component.
 * \param blue Blue color component.
 */
void NEOSR_SetHandColor(NEOSR_Handle_t device, uint8_t red, uint8_t green, uint8_t blue);

#if PL_CONFIG_USE_LED_DIMMING
/*!
 * \brief Sets the main hand brightness.
 * \param device NeoStepperRing device handle.
 * \param brightness Brightness value.
 */
  void NEOSR_SetHandBrightness(NEOSR_Handle_t device, uint8_t brightness);
/*!
 * \brief Starts dimming the main hand toward a target brightness.
 * \param device NeoStepperRing device handle.
 * \param targetBrightness Target brightness for dimming.
 */
  void NEOSR_StartHandDimming(NEOSR_Handle_t device, uint8_t targetBrightness);
/*!
 * \brief Checks whether hand dimming is still in progress.
 * \param device NeoStepperRing device handle.
 * \return True if the condition or operation succeeds, false otherwise.
 */
  bool NEOSR_HandDimmingNotFinished(NEOSR_Handle_t device);
#endif

#if PL_CONFIG_USE_EXTENDED_HANDS
/*!
 * \brief Enables or disables the second hand LED for a NeoStepperRing device.
 * \param device NeoStepperRing device handle.
 * \param on True to enable or turn on, false to disable or turn off.
 */
  void NEOSR_Set2ndHandLedEnabled(NEOSR_Handle_t device, bool on);
/*!
 * \brief Returns whether the second hand LED is enabled.
 * \param device NeoStepperRing device handle.
 * \return True if the condition or operation succeeds, false otherwise.
 */
  bool NEOSR_Get2ndHandLedEnabled(NEOSR_Handle_t device);
/*!
 * \brief Sets the second hand LED color.
 * \param device NeoStepperRing device handle.
 * \param red Red color component.
 * \param green Green color component.
 * \param blue Blue color component.
 */
  void NEOSR_Set2ndHandColor(NEOSR_Handle_t device, uint8_t red, uint8_t green, uint8_t blue);
#endif

/*!
 * \brief Enables or disables the LED ring.
 * \param device NeoStepperRing device handle.
 * \param on True to enable or turn on, false to disable or turn off.
 */
void NEOSR_SetRingLedEnabled(NEOSR_Handle_t device, bool on);
/*!
 * \brief Returns whether the LED ring is enabled.
 * \param device NeoStepperRing device handle.
 * \return True if the condition or operation succeeds, false otherwise.
 */
bool NEOSR_GetRingLedEnabled(NEOSR_Handle_t device);
/*!
 * \brief Sets the color of one pixel in the LED ring.
 * \param device NeoStepperRing device handle.
 * \param pos Pixel or step position.
 * \param red Red color component.
 * \param green Green color component.
 * \param blue Blue color component.
 */
void NEOSR_SetRingPixelColor(NEOSR_Handle_t device, uint8_t pos, uint8_t red, uint8_t green, uint8_t blue);
/*!
 * \brief Sets the color of all pixels in the LED ring.
 * \param device NeoStepperRing device handle.
 * \param red Red color component.
 * \param green Green color component.
 * \param blue Blue color component.
 */
void NEOSR_SetRingColor(NEOSR_Handle_t device, uint8_t red, uint8_t green, uint8_t blue);

/*!
 * \brief Illuminates an LED ring position for a stepper hand position.
 * \param stepperPos Current stepper position.
 * \param ledStartPos First LED position used for illumination.
 * \param ledLane NeoPixel lane index.
 * \param cw True for clockwise LED ordering, false for counter-clockwise.
 * \param ledRed Red color component.
 * \param ledGreen Green color component.
 * \param ledBlue Blue color component.
 */
void NEOSR_IlluminatePos(int stepperPos, int ledStartPos, int ledLane, bool cw, int ledRed, int ledGreen, int ledBlue);

/*!
 * \brief Appends ring status text to a buffer.
 * \param device NeoStepperRing device handle.
 * \param buf Data or text buffer.
 * \param bufSize Size of the buffer in bytes.
 */
void NEOSR_StrCatRingStatus(NEOSR_Handle_t device, unsigned char *buf, size_t bufSize);
/*!
 * \brief Appends hand status text to a buffer.
 * \param device NeoStepperRing device handle.
 * \param buf Data or text buffer.
 * \param bufSize Size of the buffer in bytes.
 */
void NEOSR_StrCatHandStatus(NEOSR_Handle_t device, unsigned char *buf, size_t bufSize);

/*!
 * \brief Deinitializes NeoStepperRing support.
 */
void NEOSR_Deinit(void);
/*!
 * \brief Initializes NeoStepperRing support.
 */
void NEOSR_Init(void);

#endif /* NEOSTEPPERRING_H_ */
