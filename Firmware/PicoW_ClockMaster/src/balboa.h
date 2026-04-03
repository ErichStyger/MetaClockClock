/*
 * Copyright (c) 2025, Erich Styger
 *
 * SPDX-License-Identifier: BSD-3-Clause
 */

#ifndef __BALBOA_H
#define __BALBOA_H

#ifdef __cplusplus
extern "C" {
#endif

#include <stdbool.h>
#include "McuShell.h"

/*!
 * \brief Command line and shell handler
 * \param cmd The command to be parsed
 * \param handled If command has been recognized and handled
 * \param io I/O handler to be used
 * \return error code, otherwise ERR_OK
 */
uint8_t Balboa_ParseCommand(const unsigned char* cmd, bool *handled, const McuShell_StdIOType *io);

float Balboa_GetCurrentTemperature(void);
float Balboa_GetSetTemperature(void);
void Balboa_SetTemperature(float temperature);

bool Balboa_GetLightIsOn(void);
void Balboa_SetLightIsOn(bool on);

bool Balboa_GetLowTempRangeIsOn(void);
void Balboa_SetLowTempRangeIsOn(bool on);

bool Balboa_GetPump1IsOn(void);
void Balboa_SetPump1IsOn(bool on);

bool Balboa_GetPump2IsOn(void);
void Balboa_SetPump2IsOn(bool on);

bool Balboa_GetPumpCirculationIsOn(void);

void Balboa_SetTime(uint8_t hours, uint8_t minutes);
void Balboa_GetTime(uint8_t *pHours, uint8_t *pMinutes);

bool Balboa_GetHeatingReadyIsOn(void);
void Balboa_SetHeatingReadyIsOn(bool on);

void Balboa_GetFilterCycle(uint8_t idx, bool *enabled, uint8_t *startHour, uint8_t *startMinute, uint8_t *endHour, uint8_t *endMinute);
void Balboa_SetFilterCycle2OnOff(bool on);

void Balboa_SetFilterCycle1StartTime(uint8_t hour, uint8_t minute);
void Balboa_SetFilterCycle2StartTime(uint8_t hour, uint8_t minute);

void Balboa_SetFilterCycle1EndTime(uint8_t hour, uint8_t minute);
void Balboa_SetFilterCycle2EndTime(uint8_t hour, uint8_t minute);

void Balboa_SetPicoReboot(bool on);

/*!
* \brief Module de-initialization
 */
void Balboa_Deinit(void);

/*!
* \brief Module initialization
 */
void Balboa_Init(void);

#ifdef __cplusplus
}  /* extern "C" */
#endif

#endif /* __BALBOA_H */