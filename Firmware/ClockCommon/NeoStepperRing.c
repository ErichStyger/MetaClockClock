/*
 * Copyright (c) 2020, Erich Styger
 *
 * SPDX-License-Identifier: BSD-3-Clause
 */

#include "platform.h"
#if PL_CONFIG_USE_LED_RING
#include "NeoStepperRingConfig.h"
#include "NeoStepperRing.h"
#include "stepper.h"
#include "NeoPixel.h"
#include <stddef.h>
#include <stdlib.h>
#include <assert.h>
#include <string.h>
#if NEOSR_CONFIG_USE_FREERTOS_HEAP
  #include "McuRTOS.h"
#endif

#define NEOSR_NOF_360        (STEPPER_CLOCK_360_STEPS) /* number of steps for 360 degree */
#define NEOSR_STEPS_FOR_LED  (NEOSR_NOF_360/NEOSR_NOF_RING_LED) /* number of steps for each LED */
#define NEOSR_LED_WIDTH      (NEOSR_STEPS_FOR_LED /*+ 50*/) /* width which shall be illuminated for the hand */

/* default configuration, used for initializing the config */
static const NEOSR_Config_t defaultConfig =
{
  .ledCw = true,
  .ledLane = 0,
  .ledStartPos = 0,
  .hand.enabled = false,
  .hand.red = 0,
  .hand.blue = 0,
  .hand.green = 0,
  .ring.enabled = false,
  .ring.red = 0,
  .ring.green = 0,
  .ring.blue = 0,
};

/* device for a single LED ring */
typedef struct {
  bool ledCw;      /* clock-wise */
  int ledLane;     /* LED lane */
  int ledStartPos; /* LED starting position in lane */
  struct {
    bool enabled; /* if hand is on or off */
    uint8_t red, green, blue; /* colors */
#if PL_CONFIG_USE_LED_DIMMING
    uint8_t brightness; /* current brightness, 0-255 */
    uint8_t targetBrightness; /* value higher or lower than brightness: will gradually change */
#endif
  } hand;
#if PL_CONFIG_USE_DUAL_HANDS
  struct {
    bool enabled; /* if LED is on or off */
    uint8_t red, green, blue; /* colors */
  } hand2nd;
#endif
  struct {
    bool enabled; /* if ring is on or off */
    uint8_t red, green, blue; /* colors */
  } ring;
} NEOSR_Device_t;

void NEOSR_GetDefaultConfig(NEOSR_Config_t *config) {
  memcpy(config, &defaultConfig, sizeof(*config));
}

NEOSR_Handle_t NEOSR_DeinitDevice(NEOSR_Handle_t device) {
#if NEOSR_CONFIG_USE_FREERTOS_HEAP
  vPortFree(device);
#else
  free(device);
#endif
  return NULL;
}

NEOSR_Handle_t NEOSR_InitDevice(NEOSR_Config_t *config) {
  NEOSR_Device_t *handle;

#if NEOSR_CONFIG_USE_FREERTOS_HEAP
  handle = (NEOSR_Device_t*)pvPortMalloc(sizeof(NEOSR_Device_t)); /* get a new device descriptor */
#else
  handle = (NEOSR_Device_t*)malloc(sizeof(NEOSR_Device_t)); /* get a new device descriptor */
#endif
  assert(handle!=NULL);
  if (handle!=NULL) { /* if malloc failed, will return NULL pointer */
    memset(handle, 0, sizeof(NEOSR_Device_t)); /* init all fields */
    handle->ledCw = config->ledCw;
    handle->ledLane = config->ledLane;
    handle->ledStartPos = config->ledStartPos;
    handle->hand.enabled = config->hand.enabled;
    handle->hand.red = config->hand.red;
    handle->hand.green = config->hand.green;
    handle->hand.blue = config->hand.blue;
#if PL_CONFIG_USE_DUAL_HANDS
    handle->hand2nd.enabled = config->hand.enabled;
    handle->hand2nd.red = config->hand.red;
    handle->hand2nd.green = config->hand.green;
    handle->hand2nd.blue = config->hand.blue;
#endif
    handle->ring.enabled = config->ring.enabled;
    handle->ring.red = config->ring.red;
    handle->ring.green = config->ring.green;
    handle->ring.blue = config->ring.blue;
  }
  return handle;
}

/* return a scale between 0 and 0xff, depending on how much there is an overlap */
static int ScaleRange(int ledPos, int startPos, int endPos) {
  int percentage, diff;
  int a, b;

  if (startPos<0) {
    startPos += STEPPER_CLOCK_360_STEPS; /* make it positive */
    endPos += STEPPER_CLOCK_360_STEPS; /* adjust end pos too */
    ledPos += NEOSR_NOF_RING_LED;
  }
  if (ledPos<0) {
    ledPos = NEOSR_NOF_RING_LED+ledPos;
  }
  /* calculate start and end position of LED */
  a = ledPos*NEOSR_STEPS_FOR_LED;
  b = a+NEOSR_STEPS_FOR_LED-1;
  /* match start and end to a and b */
  if (a>=startPos && a<=endPos) {
    startPos = a;
  }
  if (b>=startPos && b<=endPos) {
    endPos = b;
  }
  if (b>=startPos && a<=endPos) { /* they do overlap */
    diff = endPos-startPos+1;
    percentage = diff*0xff/NEOSR_STEPS_FOR_LED;
  } else {
    percentage = 0; /* no overlap */
  }
  return percentage;
}

void NEOSR_IlluminatePos(int stepperPos, int ledLane, int ledStartPos, bool cw, int ledRed, int ledGreen, int ledBlue) {
  int ledPos, pos;
  int dist[3];

  stepperPos += NEOSR_STEPS_FOR_LED/2; /* adjust by half a LED, because 0 is at the middle of the 12-o-clock LED */
  /* make stepperPos fit within 0...NEOSR_NOF_360 */
  if (stepperPos<0) {
    stepperPos = -stepperPos;
    stepperPos %= NEOSR_NOF_360;
    stepperPos = NEOSR_NOF_360-stepperPos;
  }
  /* stepperPos is now positive */
  stepperPos %= NEOSR_NOF_360;
  /* stepperPos is now within 0..NEOSR_NOF_360 */
  ledPos = stepperPos/NEOSR_STEPS_FOR_LED;
  /*
   *  stepper pos  ... 4319 | 0 107 | 108 | ...
   *  led pos                    53|54-161 | 162..(+108) |
   *  led index        39       0        1
   */
  dist[0] = ScaleRange(ledPos-1, stepperPos-NEOSR_LED_WIDTH/2, stepperPos+NEOSR_LED_WIDTH/2-1);
  dist[1] = ScaleRange(ledPos, stepperPos-NEOSR_LED_WIDTH/2, stepperPos+NEOSR_LED_WIDTH/2-1);
  dist[2] = ScaleRange(ledPos+1, stepperPos-NEOSR_LED_WIDTH/2, stepperPos+NEOSR_LED_WIDTH/2-1);

  uint8_t r,g,b;

#if NEOSR_CONFIG_USE_GAMMA_CORRECTION
  /* perform a gamma correction */
  r = NEO_GammaCorrect8(ledRed*dist[0]/0xff);
  g = NEO_GammaCorrect8(ledGreen*dist[0]/0xff);
  b = NEO_GammaCorrect8(ledBlue*dist[0]/0xff);
#else
  r = ledRed*dist[0]/0xff;
  g = ledGreen*dist[0]/0xff;
  b = ledBlue*dist[0]/0xff;
#endif
  pos = ledPos-1;
  if (pos<0) {
    pos = NEOSR_NOF_RING_LED-1;
  }
  if (!cw && pos!=0) { /* counter-clockwise order of LEDs */
    pos = NEOSR_NOF_RING_LED-pos;
  }
  //NEO_OrPixelRGB(ledLane, ledStartPos+pos, r, g, b);
  if (r!=0 || g!=0 || b!=0) {
    NEO_SetPixelRGB(ledLane, ledStartPos+pos, r, g, b);
  }
#if NEOSR_CONFIG_USE_GAMMA_CORRECTION
  r = NEO_GammaCorrect8(ledRed*dist[1]/0xff);
  g = NEO_GammaCorrect8(ledGreen*dist[1]/0xff);
  b = NEO_GammaCorrect8(ledBlue*dist[1]/0xff);
#else
  r = ledRed*dist[1]/0xff;
  g = ledGreen*dist[1]/0xff;
  b = ledBlue*dist[1]/0xff;
#endif
  pos = ledPos;
  if (!cw && pos!=0) { /* counter-clockwise order of LEDs */
    pos = NEOSR_NOF_RING_LED-pos;
  }
  //NEO_OrPixelRGB(ledLane, ledStartPos+pos, r, g, b);
  if (r!=0 || g!=0 || b!=0) {
    NEO_SetPixelRGB(ledLane, ledStartPos+pos, r, g, b);
  }
#if NEOSR_CONFIG_USE_GAMMA_CORRECTION
  r = NEO_GammaCorrect8(ledRed*dist[2]/0xff);
  g = NEO_GammaCorrect8(ledGreen*dist[2]/0xff);
  b = NEO_GammaCorrect8(ledBlue*dist[2]/0xff);
#else
  r = ledRed*dist[2]/0xff;
  g = ledGreen*dist[2]/0xff;
  b = ledBlue*dist[2]/0xff;
#endif
  pos = ledPos+1;
  if (pos>NEOSR_NOF_RING_LED-1) {
    pos = 0;
  }
  if (!cw && pos!=0) { /* counter-clockwise order of LEDs */
    pos = NEOSR_NOF_RING_LED-pos;
  }
  //NEO_OrPixelRGB(ledLane, ledStartPos+pos, r, g, b);
  if (r!=0 || g!=0 || b!=0) {
    NEO_SetPixelRGB(ledLane, ledStartPos+pos, r, g, b);
  }
}

void NEOSR_SetRingPixelColor(NEOSR_Handle_t device, uint8_t pos, uint8_t red, uint8_t green, uint8_t blue) {
  NEOSR_Device_t *dev = (NEOSR_Device_t*)device;

  NEO_SetPixelRGB(dev->ledLane, dev->ledStartPos+pos, red, green, blue);
}

void NEOSR_OrRingPixelColor(NEOSR_Handle_t device, uint8_t pos, uint8_t red, uint8_t green, uint8_t blue) {
  NEOSR_Device_t *dev = (NEOSR_Device_t*)device;

  NEO_OrPixelRGB(dev->ledLane, dev->ledStartPos+pos, red, green, blue);
}


void NEOSR_SetRingColor(NEOSR_Handle_t device, uint8_t red, uint8_t green, uint8_t blue) {
  NEOSR_Device_t *dev = (NEOSR_Device_t*)device;

  dev->ring.red = red;
  dev->ring.green = green;
  dev->ring.blue = blue;
}

void NEOSR_Illuminate(NEOSR_Handle_t device, int32_t stepperPos) {
  NEOSR_Device_t *dev = (NEOSR_Device_t*)device;
  uint8_t r, g, b;

  if (dev->ring.enabled && (dev->ring.red!=0 || dev->ring.green!=0 || dev->ring.blue!=0)) {
    for(uint8_t pos=0; pos<NEOSR_NOF_RING_LED; pos++) {
      NEOSR_OrRingPixelColor(device, pos, dev->ring.red, dev->ring.green, dev->ring.blue);
    }
  }
  if (dev->hand.enabled) {
    r = dev->hand.red;
    g = dev->hand.green;
    b = dev->hand.blue;
#if PL_CONFIG_USE_LED_DIMMING
    if (dev->hand.brightness < dev->hand.targetBrightness) {
      dev->hand.brightness++;
    } else if (dev->hand.brightness > dev->hand.targetBrightness) {
      dev->hand.brightness--;
    }
    r = (((int)r)*dev->hand.brightness)/255;
    g = (((int)g)*dev->hand.brightness)/255;
    b = (((int)b)*dev->hand.brightness)/255;
#endif
    NEOSR_IlluminatePos(stepperPos, dev->ledLane, dev->ledStartPos, dev->ledCw, r, g, b);
  }
#if PL_CONFIG_USE_DUAL_HANDS
  if (dev->hand2nd.enabled) {
    stepperPos += STEPPER_CLOCK_360_STEPS/2; /* extra 180 degree */
    NEOSR_IlluminatePos(stepperPos, dev->ledLane, dev->ledStartPos, dev->ledCw, dev->hand2nd.red, dev->hand2nd.green, dev->hand2nd.blue);
  }
#endif
}

#if PL_CONFIG_USE_DUAL_HANDS
void NEOSR_Set2ndHandColor(NEOSR_Handle_t device, uint8_t red, uint8_t green, uint8_t blue) {
  NEOSR_Device_t *dev = (NEOSR_Device_t*)device;

  dev->hand2nd.red = red;
  dev->hand2nd.green = green;
  dev->hand2nd.blue = blue;
}

void NEOSR_Set2ndHandLedEnabled(NEOSR_Handle_t device, bool on) {
  NEOSR_Device_t *dev = (NEOSR_Device_t*)device;
  dev->hand2nd.enabled = on;
}

bool NEOSR_Get2ndHandLedEnabled(NEOSR_Handle_t device) {
  NEOSR_Device_t *dev = (NEOSR_Device_t*)device;
  return dev->hand2nd.enabled;
}
#endif /* PL_CONFIG_USE_DUAL_HANDS */

void NEOSR_SetHandColor(NEOSR_Handle_t device, uint8_t red, uint8_t green, uint8_t blue) {
  NEOSR_Device_t *dev = (NEOSR_Device_t*)device;

  dev->hand.red = red;
  dev->hand.green = green;
  dev->hand.blue = blue;
}

#if PL_CONFIG_USE_LED_DIMMING
void NEOSR_SetHandBrightness(NEOSR_Handle_t device, uint8_t brightness) {
  NEOSR_Device_t *dev = (NEOSR_Device_t*)device;

  dev->hand.brightness = brightness;
  dev->hand.targetBrightness = brightness;
}
#endif

#if PL_CONFIG_USE_LED_DIMMING
void NEOSR_StartHandDimming(NEOSR_Handle_t device, uint8_t targetBrightness) {
  NEOSR_Device_t *dev = (NEOSR_Device_t*)device;

  if (dev->hand.brightness != targetBrightness) {
    dev->hand.enabled = true; /* turn on */
    dev->hand.targetBrightness = targetBrightness; /* set target */
  }
}
#endif

#if PL_CONFIG_USE_LED_DIMMING
bool NEOSR_HandDimmingNotFinished(NEOSR_Handle_t device) {
  NEOSR_Device_t *dev = (NEOSR_Device_t*)device;

  return (dev->hand.brightness != dev->hand.targetBrightness);
}
#endif

void NEOSR_SetHandLedEnabled(NEOSR_Handle_t device, bool on) {
  NEOSR_Device_t *dev = (NEOSR_Device_t*)device;
  dev->hand.enabled = on;
}

bool NEOSR_GetHandLedEnabled(NEOSR_Handle_t device) {
  NEOSR_Device_t *dev = (NEOSR_Device_t*)device;
  return dev->hand.enabled;
}

void NEOSR_SetRingLedEnabled(NEOSR_Handle_t device, bool on) {
  NEOSR_Device_t *dev = (NEOSR_Device_t*)device;
  dev->ring.enabled = on;
}

bool NEOSR_GetRingLedEnabled(NEOSR_Handle_t device) {
  NEOSR_Device_t *dev = (NEOSR_Device_t*)device;
  return dev->ring.enabled;
}

void NEOSR_Deinit(void) {
  /* nothing needed */
}

void NEOSR_Init(void) {
  /* nothing needed */
}

#endif /* PL_CONFIG_USE_LED_RING */
