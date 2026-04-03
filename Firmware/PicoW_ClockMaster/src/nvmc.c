/*
 * Copyright (c) 2019-2022, Erich Styger
 *
 * SPDX-License-Identifier: BSD-3-Clause
 */

#include "platform.h"
#if PL_CONFIG_USE_NVMC
#include "nvmc.h"
#include <string.h>
#if PL_CONFIG_USE_MININI
  #include "MinIni/McuMinINI.h"
#endif
#include "McuLib.h"
#include "McuUtility.h"
#include "shell.h"
#include "McuLog.h"

#define NVMC_MININI_FILE_NAME         "settings.ini" /* 'file' name used */

/* RS-485 bus settings */
#define NVMC_MININI_SECTION_RS485     "rs485"
#define NVMC_MININI_KEY_RS485_ADDR    "addr" /* long value: RS-485 address */

/* application settings */
#define NVMC_MININI_SECTION_APP       "app"
#define NVMC_MININI_KEY_APP_FLAGS     "flags" /* long value: application flags */

/* motor settings */
#define NVMC_MININI_SECTION_MOTOR     "motor" /* motor0, motor1, ... */
#define NVMC_MININI_KEY_MOTOR_ID      "id"    /* long value: stepper motor id/label */
#define NVMC_MININI_KEY_MOTOR_OFFSET  "offs"  /* long value: stepper motor offset */
#define NVMC_MININI_KEY_MOTOR_X       "x"     /* long value: x position of split-flap */
#define NVMC_MININI_KEY_MOTOR_Y       "y"     /* long value: y position of split-flap */

uint8_t NVMC_GetRS485Addr(uint8_t *addr) {
#if PL_CONFIG_USE_MININI
  *addr = McuMinINI_ini_getl(NVMC_MININI_SECTION_RS485, NVMC_MININI_KEY_RS485_ADDR, 0x1, NVMC_MININI_FILE_NAME);
  return ERR_OK;
#else
  *addr = 0x1;
  return ERR_OK;
#endif
}

uint8_t NVMC_SetRS485Addr(uint8_t addr) {
#if PL_CONFIG_USE_MININI && McuMinINI_CONFIG_READ_ONLY
  return ERR_FAILED;
#elif PL_CONFIG_USE_MININI
  McuMinINI_ini_putl(NVMC_MININI_SECTION_RS485, NVMC_MININI_KEY_RS485_ADDR, addr, NVMC_MININI_FILE_NAME);
  return ERR_OK;
#else
  return ERR_FAILED;
#endif
}

uint8_t NVMC_GetFlags(uint32_t *flags) {
#if PL_CONFIG_USE_MININI
  *flags = McuMinINI_ini_getl(NVMC_MININI_SECTION_APP, NVMC_MININI_KEY_APP_FLAGS, 0x0, NVMC_MININI_FILE_NAME);
  return ERR_OK;
#else
  *flags = 0;
  return ERR_OK;
#endif
}

uint8_t NVMC_SetFlags(uint32_t flags) {
#if PL_CONFIG_USE_MININI && McuMinINI_CONFIG_READ_ONLY
  return ERR_FAILED;
#elif PL_CONFIG_USE_MININI
  McuMinINI_ini_putl(NVMC_MININI_SECTION_APP, NVMC_MININI_KEY_APP_FLAGS, flags, NVMC_MININI_FILE_NAME);
  return ERR_OK;
#else
  return ERR_FAILED;
#endif
}

#if PL_CONFIG_USE_MININI
static void BuildStepperSectionName(unsigned char *sectionName, size_t nameSize, uint8_t motorIdx) {
  McuUtility_strcpy(sectionName, nameSize, (unsigned char*)NVMC_MININI_SECTION_MOTOR);
  McuUtility_strcatNum8u(sectionName, nameSize, motorIdx); /* append motor index to section name */
}
#endif

uint8_t NVMC_GetStepperID(uint8_t motorIdx, int16_t *id) {
#if PL_CONFIG_USE_MININI
  unsigned char sectionName[16];

  BuildStepperSectionName(sectionName, sizeof(sectionName), motorIdx);
  *id = McuMinINI_ini_getl((char*)sectionName, NVMC_MININI_KEY_MOTOR_ID, -1, NVMC_MININI_FILE_NAME);
  return ERR_OK;
#else
  *id = -1;
  return ERR_OK;
#endif
}

uint8_t NVMC_SetStepperID(uint8_t motorIdx, uint16_t id) {
#if PL_CONFIG_USE_MININI && McuMinINI_CONFIG_READ_ONLY
  return ERR_FAILED;
#elif PL_CONFIG_USE_MININI
  unsigned char sectionName[16];

  BuildStepperSectionName(sectionName, sizeof(sectionName), motorIdx);
  McuMinINI_ini_putl((char*)sectionName, NVMC_MININI_KEY_MOTOR_ID, id, NVMC_MININI_FILE_NAME);
  return ERR_OK;
#else
  return ERR_FAILED;
#endif
}

uint8_t NVMC_GetStepperPosition(uint8_t motorIdx, int16_t *xp, int16_t *yp) {
#if PL_CONFIG_USE_MININI
  unsigned char sectionName[16];

  BuildStepperSectionName(sectionName, sizeof(sectionName), motorIdx);
  *xp = McuMinINI_ini_getl((char*)sectionName, NVMC_MININI_KEY_MOTOR_X, -1, NVMC_MININI_FILE_NAME);
  *yp = McuMinINI_ini_getl((char*)sectionName, NVMC_MININI_KEY_MOTOR_Y, -1, NVMC_MININI_FILE_NAME);
  return ERR_OK;
#else
  *xp = 0;
  *yp = 0;
  return ERR_OK;
#endif
}

uint8_t NVMC_SetStepperPosition(uint8_t motorIdx, int16_t xPos, int16_t yPos) {
#if PL_CONFIG_USE_MININI && McuMinINI_CONFIG_READ_ONLY
  return ERR_FAILED;
#elif PL_CONFIG_USE_MININI
  unsigned char sectionName[16];

  BuildStepperSectionName(sectionName, sizeof(sectionName), motorIdx);
  McuMinINI_ini_putl((char*)sectionName, NVMC_MININI_KEY_MOTOR_X, xPos, NVMC_MININI_FILE_NAME);
  McuMinINI_ini_putl((char*)sectionName, NVMC_MININI_KEY_MOTOR_Y, yPos, NVMC_MININI_FILE_NAME);
  return ERR_OK;
#else
  return ERR_FAILED;
#endif
}

int16_t NVMC_GetStepperZeroOffset(uint8_t motorIdx) {
#if PL_CONFIG_USE_MININI
  unsigned char sectionName[16];

  BuildStepperSectionName(sectionName, sizeof(sectionName), motorIdx);
  return McuMinINI_ini_getl((char*)sectionName, NVMC_MININI_KEY_MOTOR_OFFSET, 0, NVMC_MININI_FILE_NAME);
#else
  return 0;
#endif
}

uint8_t NVMC_SetStepperZeroOffset(uint8_t motorIdx, int16_t offset) {
#if PL_CONFIG_USE_MININI && McuMinINI_CONFIG_READ_ONLY
  return ERR_FAILED;
#elif PL_CONFIG_USE_MININI
  unsigned char sectionName[16];

  BuildStepperSectionName(sectionName, sizeof(sectionName), motorIdx);
  McuMinINI_ini_putl((char*)sectionName, NVMC_MININI_KEY_MOTOR_OFFSET, offset, NVMC_MININI_FILE_NAME);
  return ERR_OK;
#else
  return ERR_FAILED;
#endif
}

static uint8_t PrintStatus(const McuShell_StdIOType *io) {
  uint8_t buf[64];
  uint8_t res, addr;
  uint32_t flags;

  McuShell_SendStatusStr((unsigned char*)"nvmc", (unsigned char*)"Non-volatile memory configuration area\r\n", io->stdOut);

  res = NVMC_GetRS485Addr(&addr);
  if (res==ERR_OK) {
    McuUtility_strcpy(buf, sizeof(buf), (unsigned char*)"0x");
    McuUtility_strcatNum8Hex(buf, sizeof(buf), addr);
    McuUtility_strcat(buf, sizeof(buf), (unsigned char*)"\r\n");
  } else {
    McuUtility_strcpy(buf, sizeof(buf), (unsigned char*)"ERROR\r\n");
  }
  McuShell_SendStatusStr((unsigned char*)"  RS-485 addr", buf, io->stdOut);

  res = NVMC_GetFlags(&flags);
  if (res==ERR_OK) {
    McuUtility_strcpy(buf, sizeof(buf), (unsigned char*)"0x");
    McuUtility_strcatNum32Hex(buf, sizeof(buf), flags);
    McuUtility_strcat(buf, sizeof(buf), (unsigned char*)" mag: ");
    if (flags&NVMC_FLAGS_MAGNET_ENABLED) {
      McuUtility_strcat(buf, sizeof(buf), (unsigned char*)"1");
    } else {
      McuUtility_strcat(buf, sizeof(buf), (unsigned char*)"0");
    }
    McuUtility_strcat(buf, sizeof(buf), (unsigned char*)"\r\n");
  } else {
    McuUtility_strcpy(buf, sizeof(buf), (unsigned char*)"ERROR\r\n");
  }
  McuShell_SendStatusStr((unsigned char*)"  flags", buf, io->stdOut);
  return ERR_OK;
}

static uint8_t PrintHelp(const McuShell_StdIOType *io) {
  McuShell_SendHelpStr((unsigned char*)"nvmc", (unsigned char*)"Group of NVMC commands\r\n", io->stdOut);
  McuShell_SendHelpStr((unsigned char*)"  help|status", (unsigned char*)"Print help or status information\r\n", io->stdOut);
  McuShell_SendHelpStr((unsigned char*)"  flags <val>", (unsigned char*)"Set flags\r\n", io->stdOut);
  return ERR_OK;
}

uint8_t NVMC_ParseCommand(const unsigned char *cmd, bool *handled, const McuShell_StdIOType *io) {
  if (McuUtility_strcmp((char*)cmd, McuShell_CMD_HELP)==0 || McuUtility_strcmp((char*)cmd, "nvmc help")==0) {
    *handled = true;
    return PrintHelp(io);
  } else if ((McuUtility_strcmp((char*)cmd, McuShell_CMD_STATUS)==0) || (McuUtility_strcmp((char*)cmd, "nvmc status")==0)) {
    *handled = true;
    return PrintStatus(io);
  } else if (McuUtility_strncmp((char*)cmd, "nvmc flags", sizeof("nvmc flags")-1)==0) {
    *handled = true;
    const unsigned char *p;
    int32_t val;

    p = cmd + sizeof("nvmc flags ")-1;
    if (McuUtility_xatoi(&p, &val)==ERR_OK) {
       return NVMC_SetFlags((uint32_t)val);
    } else {
      return ERR_FAILED;
    }
  }
  return ERR_OK;
}

void NVMC_Deinit(void) {
  /* nothing needed */
}

void NVMC_Init(void){
  /* nothing needed */
}
#endif /* PL_CONFIG_USE_NVMC */
