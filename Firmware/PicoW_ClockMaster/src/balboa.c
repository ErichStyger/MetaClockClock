/*
 * Copyright (c) 2025, Erich Styger
 *
 * SPDX-License-Identifier: BSD-3-Clause
 */

#include "platform.h"
#if PL_CONFIG_USE_BALBOA
#include "balboa.h"
#include "McuRTOS.h"
#include "McuLog.h"
#include "McuUtility.h"
#include "McuUart485.h"
#include "McuArmTools.h"
#include "McuTimeDate.h"
#include "leds.h"
#if PL_CONFIG_USE_MININI
  #include "minIni/McuMinINI.h"
  #include "MinIniKeys.h"
#endif
#if PL_CONFIG_USE_WATCHDOG
  #include "McuWatchdog.h"
#endif

#if !McuUart485_CONFIG_USE_RAW
  #error "This module is only for raw mode!"
#endif

typedef enum toggle_item_e {
  Toggle_Item_Normal_Mode=0x01, 	/* Normal Operation (exit Priming Mode, etc.) */
  Toggle_Item_Clear_Notification=0x03, 	/* Clear notification (reminders, etc.) */
  Toggle_Item_Pump1=0x04, 	/* Pump 1 */
  Toggle_Item_Pump2=0x05, 	/* Pump 2 */
  Toggle_Item_Pump3=0x06, 	/* Pump 3 */
  Toggle_Item_Pump4=0x07, 	/* Pump 4 */
  Toggle_Item_Pump5=0x08, 	/* Pump 5 */
  Toggle_Item_Pump6=0x09, 	/* Pump 6 */
  Toggle_Item_Blower=0x0C, 	/* Blower */
  Toggle_Item_Mister=0x0E, 	/* Mister */
  Toggle_Item_Light1=0x11, 	/* Light 1 */
  Toggle_Item_Light2=0x12, 	/* Light 2 */
  Toggle_Item_Aux1=0x16, 	/* Aux 1 */
  Toggle_Item_Aux2=0x17, 	/* Aux 2 */
  Toggle_Item_Soak_Mode=0x1D, 	/* Soak Mode (All pumps off) */
  Toggle_Item_Hold_Mode=0x3C, 	/* Hold Mode */
  Toggle_Item_Temp_Range=0x50, 	/* Temperature Range (Low/High) */
  Toggle_Item_Heat_Mode=0x51, 	/* Heat Mode (Ready/Rest) */
} toggle_item_e;

typedef enum request_item_e {
  Request_Item_Configuration  = 0x00,
  Request_Item_FilterCycles   = 0x01,
  Request_Item_Information    = 0x02,
  Request_Item_Preferences    = 0x08,
  Request_Item_FaultLog       = 0x20,
  Request_Item_GFCITest       = 0x80,
} request_item_e;

typedef enum message_kind_e {
  Message_Kind_Toggle,          /* message to toggle an item */
  Message_Kind_SetTime,         /* message to sent the time */
  Message_Kind_Request,         /* message to request information */
  Message_Kind_FilterCycles,    /* message sent to the main board with new filter cycle information */
  Message_Kind_SetTemperature,  /* message to set a new temperature */
} message_kind_e;

typedef struct BalboaFilterCycles_t {
  bool isValid;
  struct {
    bool isEnabled;
    uint8_t startHour, startMinute;
    uint8_t durationHours, durationMinutes;
  } cycles[2];
} BalboaFilterCycles_t;

typedef struct message_t {
  message_kind_e kind;
  union u_ {
    struct { /* Message_Kind_Toggle */
      toggle_item_e item;
    } toggle;
    struct { /* Message_Kind_SetTime */
      uint8_t hours, minutes;
    } setTime;
    struct { /* Message_Kind_Request */
      request_item_e item;
    } request;
    struct {
      BalboaFilterCycles_t info;
    } filters;
    struct { /* Message_Kind_SetTemperatureRequest */
      float valueCelcisus;
    } setTemperature;
  } u;
} message_t;

static QueueHandle_t messageQueue; /* queue handle */
#define MESSAGE_QUEUE_LENGTH    (32) /* number of elements in queue */
#define MESSAGE_QUEUE_ELEM_SIZE (sizeof(message_t)) /* size of a queue item */

/* prototypes: */
static void Balboa_SetChannel(uint8_t channel);

static void Balboa_QueueMessage(message_t *msg) {
  if (uxQueueSpacesAvailable(messageQueue)<4) {
    McuLog_error("queue is getting full? Requesting new channel (power loss?)");
    Balboa_SetChannel(-1);
    return;
  }
  if (xQueueSendToBack(messageQueue, msg, portMAX_DELAY)!=pdPASS) {
    McuLog_fatal("failed to send event after timeout");
    for(;;) {}
  }
}

/* protocol: https://github.com/ccutrer/balboa_worldwide_app/wiki */
/*
Channel(s) 	Type 	    Description
0x0A 	      Unicast 	?Reserved for the WiFi Module?
0x0C 	      ?? 	      ??
0x0F 	      ?? 	      ?Only on newer Main Boards?
0x10-0x2F 	Unicast 	Assigned to clients, Main Board provides CTS
0x30-0x3F 	?Unicast? Assigned to clients, Main Board does not provide CTS
0x52 	      ?? 	      ?Only on newer Main Boards?
0x54 	      ?? 	      ?Only on newer Main Boards?
0x55 	      ?? 	      ?Only on newer Main Boards?
0xFE 	      Multicast Used by Main Board and clients for channel assignment
0xFF 	      Multicast Used by Main Board for broadcasting to all clients
*/

/* 
Message format, see https://github.com/ccutrer/balboa_worldwide_app/wiki:
Byte 	    Name 	      Description/Values
0 	      Delimiter 	0x7E
1 	      Length 	    N - 2 (minimum of 5)
2 	      Channel   	(see Channels)
3 	      ?? 	        0xAF when Byte 2 is 0xFF; 0xBF otherwise
4 	      Type Code 	(see Message Types)
5 to N-2  Arguments 	Optional, usually the same for each message type
N - 1 	  Checksum 	  CRC-8 of bytes 1 through N-2, polynomial=0x07, initial=0x02, final XOR=0x02
N 	      Delimiter 	0x7E
Example:  7E1DFFAF130000640B2B00000100000400000000000000000064000000A57E
*/

typedef struct BalboaStatus_t {
  uint8_t spaState; /* 0x00=Running, 0x01=Initializing, 0x05=Hold Mode, ?0x14=A/B Temps ON?, 0x17=Test Mode */
  uint8_t initializationMode; /* 0x00=Idle, 0x01=Priming Mode, 0x02=?Fault?, 0x03=Reminder, 0x04=?Stage 1?, 0x05=?Stage 3?, 0x42=?Stage 2? */
  uint8_t currentTemperature; /* 0xFF if unknown */
  uint8_t hour; /* 0-23 */
  uint8_t minute; /* 0-59 */
  uint8_t heatingMode; /* 0=Ready, 1=Rest, 3=Ready-in-Rest */
  uint8_t reminderType; /* 0x00=None, 0x04=Clean filter, 0x0A=Check the pH, 0x09=Check the sanitizer, 0x1E=?Fault? */
  uint8_t sensorAtemperatureHoldTimer; /* Minutes if Hold Mode else Temperature (scaled by Temperature Scale) if A/B Temps else 0x01 if Test Mode else 0x00 */
  uint8_t sensorBTemperature; /* deci-Celsius temperature if A/B Temps else 0x00 */
  uint8_t flags9, flags10, flags11, flags12, flags13, flags14;
  bool mister; /* 0=OFF, 1=ON */
  uint8_t flags18;
  uint8_t flags19;
  uint8_t setTemperature;
  uint8_t flags21;
  uint8_t m8CylceTime; /* 0=OFF; 30, 60, 90, or 120 (in minutes) */

  /* derived values for easier access: */
  bool isCelsius; /* true if Celsius, false if Fahrenheit */
  bool isLEDOn; /* if SPA LED light is on or not */
  bool isPump1On; /* if pump 1 is on or off */
  bool isPump2On; /* if pump 2 is on or off */
  bool isPumpCirculationOn; /* if circulation pump is on or off */
  bool isLowTemperatureRange; /* if temperature range is low (10-37C) or high (26.5-40C) */
} BalboaStatus_t;

#define BALBOA_NOF_MIN_MESSAGES  (10) /* number of messages received for valid data */

static struct balboa_s {
  bool on; /* on or off reading messages from RS-485 */
  uint32_t nofMessagesReceived; /* number of messages received */
  uint8_t assignedChannel; /* assigned channel after a channel request. 0xFF if not assigned */
  BalboaStatus_t statusMsg; /* last status message received*/
  BalboaFilterCycles_t filters; /* filter cycle information */
} balboa;

static uint8_t Balboa_GetChannel(void) {
  return balboa.assignedChannel;
}

static void Balboa_SetChannel(uint8_t channel) {
#if PL_CONFIG_USE_MININI
  McuMinINI_ini_putl(NVMC_MININI_SECTION_BALBOA, NVMC_MININI_KEY_BALBOA_CHANNEL, channel, NVMC_MININI_FILE_NAME);
#endif
  balboa.assignedChannel = channel;
}

static uint8_t Balboa_SetIsOn(bool enable) {
#if PL_CONFIG_USE_MININI
  if (McuMinINI_ini_putl(NVMC_MININI_SECTION_BALBOA, NVMC_MININI_KEY_BALBOA_ON, enable, NVMC_MININI_FILE_NAME)!=1) { /* 1: success */
    return ERR_FAILED;
  }
#endif
  balboa.on = enable;
  return ERR_OK;
}

static uint8_t SendMessage(const uint8_t *message, size_t size) {
  McuUart485_SendBlock(message, size);
  return ERR_OK;
}

static uint8_t calculate_crc(const uint8_t *data, size_t nofBytes) {
  /* CRC-8 of bytes 1 through N-2, polynomial=0x07, initial=0x02, final XOR=0x02 */
  #define CRC8_INITIAL_VALUE (0x02)
  #define CRC8_FINAL_XOR     (0x02)
  #define CRC8_POLYNOMIAL    (0x07)
  uint8_t flag;
  uint8_t crc = CRC8_INITIAL_VALUE;

  for (size_t i = 0; i < nofBytes; i++) {
    crc ^= data[i];
    for (uint8_t j = 0; j < 8; j++) {
      flag = crc & 0x80;
      crc <<= 1;
      if (flag) {
        crc ^= CRC8_POLYNOMIAL;
      }
    }
  }
  crc ^= CRC8_FINAL_XOR;
  return crc;
}

static uint8_t SendSettingsRequest(uint8_t channel, request_item_e request) {
  uint8_t message[10];

  message[0] = 0x7E; /* start delimiter */
  message[1] = 0x08; /* length */
  message[2] = channel;
  message[3] = 0xBF; /* 0xAF when Byte 2 is 0xFF; 0xBF otherwise */ 
  message[4] = 0x22; /* type: settings request */
  message[5] = request; /* request type */
  /* subsequent arguments */
  switch(request) {
    case Request_Item_Configuration:
      message[6] = 0x00;
      message[7] = 0x01;
      break;
    case Request_Item_FaultLog:
      message[6] = 0xFF; /* Entry Number (0-23, 0xFF is last fault) */
      message[7] = 0x00;
      break;
    default:
      message[6] = 0x00;
      message[7] = 0x00;
      break;
  } /* switch */
  message[8] = calculate_crc(&message[1], message[1]-1); /* CRC */
  message[9] = 0x7E; /* end delimiter */
  return SendMessage(message, sizeof(message));
}

static uint8_t hash1 = 0x10;
static uint8_t hash2 = 0x20;

static uint8_t SendChannelAssignmentRequest(void) {
  uint8_t message[10];

  message[0] = 0x7E; /* start delimiter */
  message[1] = 0x08; /* length */
  message[2] = 0xFE; /* multicast channel */
  message[3] = 0xBF; /* 0xAF when Byte 2 is 0xFF; 0xBF otherwise */ 
  message[4] = 0x01; /* type: channel assignment request */
  message[5] = 0x02; /* device type: 0x2 top panel? */
  message[6] = hash1; /* hash? */
  message[7] = hash2; /* hash? */
  message[8] = calculate_crc(&message[1], message[1]-1); /* CRC */
  message[9] = 0x7E; /* end delimiter */
  /* make hash unique: */
  hash1++;
  hash2++;
  return SendMessage(message, sizeof(message));
}

static uint8_t SendChannelAssignementAcknowledge(uint8_t assignedChannel) {
  uint8_t message[7];

  message[0] = 0x7E; /* start delimiter */
  message[1] = 0x05; /* length */
  message[2] = assignedChannel; /* assigned channel */
  message[3] = 0xBF; /* 0xAF when Byte 2 is 0xFF; 0xBF otherwise */ 
  message[4] = 0x03; /* type: channel assignment acknowledge */
  message[5] = calculate_crc(&message[1], message[1]-1); /* CRC */
  message[6] = 0x7E; /* end delimiter */
  return SendMessage(message, sizeof(message));
}

static uint8_t SendExistingClientResponse(void) {
  uint8_t message[10];

  message[0] = 0x7E; /* start delimiter */
  message[1] = 0x08; /* length */
  message[2] = 0x10; /* channel */
  message[3] = 0xBF; /* 0xAF when Byte 2 is 0xFF; 0xBF otherwise */ 
  message[4] = 0x05; /* type: existing client response */
  message[5] = 0x03; /* 0x03=??, 0x04=?? */
  message[6] = 0x08; /* 0x08=??, 0x37=?? */
  message[7] = 0x00; /* 0x00=??*/
  message[8] = calculate_crc(&message[1], message[1]-1); /* CRC */
  message[9] = 0x7E; /* end delimiter */
  return SendMessage(message, sizeof(message));
}

static uint8_t SendNothingToSend(uint8_t channel) {
  uint8_t message[7];

  message[0] = 0x7E; /* start delimiter */
  message[1] = 0x05; /* length */
  message[2] = channel; /* channel */
  message[3] = 0xBF; /* 0xAF when Byte 2 is 0xFF; 0xBF otherwise */ 
  message[4] = 0x07; /* type: nothing to send */
  message[5] = calculate_crc(&message[1], message[1]-1); /* CRC */
  message[6] = 0x7E; /* end delimiter */
  return SendMessage(message, sizeof(message));
}

static uint8_t SendToggleItemRequest(uint8_t channel, toggle_item_e item) {
  uint8_t message[9];

  message[0] = 0x7E; /* start delimiter */
  message[1] = 0x07; /* length */
  message[2] = channel; /* channel */
  message[3] = 0xBF; /* 0xAF when Byte 2 is 0xFF; 0xBF otherwise */ 
  message[4] = 0x11; /* type: toggle item request */
  message[5] = item;
  message[6] = 0x00;
  message[7] = calculate_crc(&message[1], message[1]-1); /* CRC */
  message[8] = 0x7E; /* end delimiter */
  return SendMessage(message, sizeof(message));
}

static uint8_t SendSetTimeRequest(uint8_t channel, uint8_t hh, uint8_t mm) {
  uint8_t message[9];

  message[0] = 0x7E; /* start delimiter */
  message[1] = 0x07; /* length */
  message[2] = channel; /* channel */
  message[3] = 0xBF; /* 0xAF when Byte 2 is 0xFF; 0xBF otherwise */ 
  message[4] = 0x21; /* type: set time request */
  message[5] = hh; /* hour */
  message[6] = mm; /* minute */
  message[7] = calculate_crc(&message[1], 6); /* CRC */
  message[8] = 0x7E; /* end delimiter */
  return SendMessage(message, sizeof(message));
}

static uint8_t SendSetTempertureRequestMessage(uint8_t channel, float temperatureCelcisus) {
  uint8_t message[8];
  uint8_t val; /* Fahrenheit: 0.1 units, Celsius: 0.5 units */

  message[0] = 0x7E; /* start delimiter */
  message[1] = 0x06; /* length */
  message[2] = channel; /* channel */
  message[3] = 0xBF; /* 0xAF when Byte 2 is 0xFF; 0xBF otherwise */ 
  message[4] = 0x20; /* type: set temperature request */
  if (balboa.statusMsg.isCelsius) {
    val = (int)(temperatureCelcisus*10.0f) / 5; /* in 0.5 degree Celsius units */
  } else {
    float fahrenheit = temperatureCelcisus*1.8f + 32.0f; /* convert from Fahrenheit to Celsius */
    val = (uint8_t)(fahrenheit/0.1f); /* in 0.1 degree Fahrenheit units */
  }
  message[5] = val; /* temperature */
  message[6] = calculate_crc(&message[1], 5); /* CRC */
  message[7] = 0x7E; /* end delimiter */
  return SendMessage(message, sizeof(message));
}

static uint8_t SendFilterCyclesMessage(uint8_t channel, BalboaFilterCycles_t *info) {
  uint8_t message[15];
//08.06.2025 17:38:42,00 TRACE balboa.c:746 ch:10 message {0x7E, 0x0D, 0x10, 0xBF, 0x23, 0x07, 0x00, 0x10, 0x00, 0x17, 0x2D, 0x02, 0x1E, 0x2A, 0x7E}

  message[0] = 0x7E; /* start delimiter */
  message[1] = 13; /* length */
  message[2] = channel; /* channel */
  message[3] = 0xBF; /* 0xAF when Byte 2 is 0xFF; 0xBF otherwise */ 
  message[4] = 0x23; /* type: filter cycle message */
  /* cycle 1 */
  message[5] = info->cycles[0].startHour;
  message[6] = info->cycles[0].startMinute;
  message[7] = info->cycles[0].durationHours;
  message[8] = info->cycles[0].durationMinutes;
  /* cycle 2 */
  message[9] = info->cycles[1].startHour; 
  if (info->cycles[1].isEnabled) {
    message[9] |= (1<<7); /* add enabled or not */
  }
  message[10] = info->cycles[1].startMinute;
  message[11] = info->cycles[1].durationHours;
  message[12] = info->cycles[1].durationMinutes;
  message[13] = calculate_crc(&message[1], 12); /* CRC */
  message[14] = 0x7E; /* end delimiter */
  return SendMessage(message, sizeof(message));
}

static const unsigned char *GetSpaStateString(uint8_t state) {
  switch(state) {
    case 0x00: return (const unsigned char*)"Running";
    case 0x01: return (const unsigned char*)"Initializing";
    case 0x05: return (const unsigned char*)"Hold Mode";
    case 0x14: return (const unsigned char*)"A/B Temps ON";
    case 0x17: return (const unsigned char*)"Test Mode";
    default: return (const unsigned char*)"Unknown";
  }
}

static const unsigned char *GetInitModeString(uint8_t mode) {
  switch(mode) {
    case 0x00: return (const unsigned char*)"Idle";
    case 0x01: return (const unsigned char*)"Priming Mode";
    case 0x02: return (const unsigned char*)"Fault";
    case 0x03: return (const unsigned char*)"Reminder";
    case 0x04: return (const unsigned char*)"Stage 1";
    case 0x05: return (const unsigned char*)"Stage 3";
    case 0x42: return (const unsigned char*)"Stage 2";
    default: return (const unsigned char*)"Unknown";
  }
}

static const unsigned char *GetHeatingModeString(uint8_t mode) {
  switch(mode) {
    case 0x00: return (const unsigned char*)"Ready (keep temperature)"; /* keep temperature in +/- 1 degree range*/
    case 0x01: return (const unsigned char*)"Rest (heat only during filter cycles)";  /* heat only during filter cycles */
    case 0x03: return (const unsigned char*)"Ready-in-Rest (heat only for 1h if pump 1 is on)"; /* same as 'rest', only heats water if necessary for 1h when you turn on pump 1 */
    default: return (const unsigned char*)"Unknown";
  }
}

static const unsigned char *GetReminderTypeString(uint8_t type) {
  switch(type) {
    case 0x00: return (const unsigned char*)"None";
    case 0x04: return (const unsigned char*)"Clean filter";
    case 0x0A: return (const unsigned char*)"Check the pH";
    case 0x09: return (const unsigned char*)"Check the sanitizer";
    case 0x1E: return (const unsigned char*)"Fault";
    default: return (const unsigned char*)"Unknown";
  }
}

static void AppendTemperatureString(bool isCelsius, uint8_t temperature, unsigned char *buf, size_t bufSize) {
  if (temperature==0xFF) {
    McuUtility_strcat(buf, bufSize, (const unsigned char*)"unknown");
  } else {
    if (isCelsius) {
      McuUtility_strcatNum8u(buf, bufSize, temperature/2);
      McuUtility_chcat(buf, bufSize, '.');
      McuUtility_strcatNum8u(buf, bufSize, (temperature%2)==0?0:5);
      McuUtility_strcat(buf, bufSize, (const unsigned char*)"C");
    } else {
      McuUtility_strcatNum8u(buf, bufSize, temperature);
      McuUtility_strcat(buf, bufSize, (const unsigned char*)"F");
    }
  }
}

static void Balboa_QueueSetTimeMessage(uint8_t hours, uint8_t minutes) {
  message_t msg;

  msg.kind = Message_Kind_SetTime;
  msg.u.setTime.hours = 0x80 | hours; /* 15h => 0x8F, high bit set for 24h format */
  msg.u.setTime.minutes = minutes;
  Balboa_QueueMessage(&msg);
}

static void Balboa_QueueSetFilterMessage(BalboaFilterCycles_t *filters) {
  message_t msg;

  msg.kind = Message_Kind_FilterCycles;
  msg.u.filters.info = *filters; /* struct copy */
  Balboa_QueueMessage(&msg);
}

static bool Balboa_SendQueuedMessage(void) {
  BaseType_t res;
  message_t msg;
  
  res = xQueueReceive(messageQueue, &msg, 0);
  if (res==pdTRUE) { /* message received */
    /* need to wait some time after the clear to send? */
  #if PL_CONFIG_USE_WATCHDOG
    McuWatchdog_DelayAndReport(McuWatchdog_REPORT_ID_TASK_BALBOA, 1, 5);
  #else
    vTaskDelay(pdMS_TO_TICKS(5));
  #endif
    switch(msg.kind) {
      case Message_Kind_Toggle:
        SendToggleItemRequest(Balboa_GetChannel(), msg.u.toggle.item);
        return true; /* message sent */
      case Message_Kind_Request:
        SendSettingsRequest(Balboa_GetChannel(), msg.u.request.item);
        return true; /* message sent */
      case Message_Kind_SetTime:
        SendSetTimeRequest(Balboa_GetChannel(), msg.u.setTime.hours, msg.u.setTime.minutes);
        return true; /* message sent */
      case Message_Kind_FilterCycles:
        SendFilterCyclesMessage(Balboa_GetChannel(), &msg.u.filters.info);
        return true;
      case Message_Kind_SetTemperature:
        SendSetTempertureRequestMessage(Balboa_GetChannel(), msg.u.setTemperature.valueCelcisus);
        return true;
      default:
        McuLog_error("wrong message kind: %d", msg.kind);
        break;
    }
  }
  return false; /* nothing sent */
}

static void Balboa_QueueRequestMessage(request_item_e item) {
  message_t msg;

  msg.kind = Message_Kind_Request;
  msg.u.request.item = item;
  Balboa_QueueMessage(&msg);
}

static void Balboa_QueueToggleMessage(toggle_item_e item) {
  message_t msg;

  msg.kind = Message_Kind_Toggle;
  msg.u.toggle.item = item;
  Balboa_QueueMessage(&msg);
}

static float ConvertTemperatureToCelsius(uint8_t value) {
  float t;

  if (value==0xff) { /* unknown */
    return 0.0f;
  }
  if (balboa.statusMsg.isCelsius) {
    t = ((float)(value/2))+(value%2)*0.5f;
  } else { /* convert Fahrenheit to Celsius */
    t = ((int)value-32) * 0.556f;
  }
  return t;
}

float Balboa_GetCurrentTemperature(void) {
  if (balboa.nofMessagesReceived<BALBOA_NOF_MIN_MESSAGES) {
    return 0.0f; /* no valid data yet */
  }
  return ConvertTemperatureToCelsius(balboa.statusMsg.currentTemperature);
}

float Balboa_GetSetTemperature(void) {
  if (balboa.nofMessagesReceived<BALBOA_NOF_MIN_MESSAGES) {
    return 0.0f; /* no valid data yet */
  }
  return ConvertTemperatureToCelsius(balboa.statusMsg.setTemperature);
}

bool Balboa_GetLightIsOn(void) {
  return balboa.statusMsg.isLEDOn;
}

void Balboa_SetLightIsOn(bool on) {
  if (on != Balboa_GetLightIsOn()) { /* state not the same? */
    Balboa_QueueToggleMessage(Toggle_Item_Light1); /* toggle light */
  }
}

bool Balboa_GetLowTempRangeIsOn(void) {
  return balboa.statusMsg.isLowTemperatureRange;
}

void Balboa_SetLowTempRangeIsOn(bool on) {
  if (on != Balboa_GetLowTempRangeIsOn()) { /* state not the same? */
    Balboa_QueueToggleMessage(Toggle_Item_Temp_Range); /* toggle temperature range */
  }
}

bool Balboa_GetPump1IsOn(void) {
  return balboa.statusMsg.isPump1On;
}

void Balboa_SetPump1IsOn(bool on) {
  if (on != Balboa_GetPump1IsOn()) { /* state not the same? */
    Balboa_QueueToggleMessage(Toggle_Item_Pump1); /* toggle pump */
  }
}

bool Balboa_GetPump2IsOn(void) {
  return balboa.statusMsg.isPump2On;
}

void Balboa_SetPump2IsOn(bool on) {
  if (on != Balboa_GetPump2IsOn()) { /* state not the same? */
    Balboa_QueueToggleMessage(Toggle_Item_Pump2); /* toggle pump */
  }
}

bool Balboa_GetPumpCirculationIsOn(void) {
  return balboa.statusMsg.isPumpCirculationOn;
}

void Balboa_GetTime(uint8_t *pHours, uint8_t *pMinutes) {
  *pHours = balboa.statusMsg.hour;
  *pMinutes = balboa.statusMsg.minute;
}

void Balboa_SetTime(uint8_t hours, uint8_t minutes) {
  Balboa_QueueSetTimeMessage(hours, minutes);
}

void Balboa_SetPicoReboot(bool on) {
  if (on) {
    McuArmTools_SoftwareReset(); /* reboot pico */
  }
}

bool Balboa_GetHeatingReadyIsOn(void){
  return balboa.statusMsg.heatingMode==0; /* 0=Ready, 1=Rest, 3=Ready-in-Rest */
}

void Balboa_SetHeatingReadyIsOn(bool on) {
  if (on != Balboa_GetHeatingReadyIsOn()) { /* state not the same? */
    Balboa_QueueToggleMessage(Toggle_Item_Heat_Mode); /* toggle heating mode (ready, rest) */
  }
}

static void CalcStartDurationToEnd(uint8_t startHour, uint8_t startMinute, uint8_t durationHours, uint8_t durationMinutes, uint8_t *endHour, uint8_t *endMinute) {
    uint32_t minutes;
    minutes =  startHour*60 + startMinute 
                + durationHours*60 + durationMinutes;
    minutes %= 24*60; /* modulo by the number of minutes of the day */
    *endHour = minutes/60;
    *endMinute = minutes%60;
}

void Balboa_GetFilterCycle(uint8_t idx, bool *enabled, uint8_t *startHour, uint8_t *startMinute, uint8_t *endHour, uint8_t *endMinute) {
  if (idx>1 || !balboa.filters.isValid) { /* index can only be zero or one, or if not valid data */
    *enabled = false;
    *startHour = 0;
    *startMinute = 0;
    *endHour = 0;
    *endMinute = 0;
    return;
  }
  *enabled= balboa.filters.cycles[idx].isEnabled;
  *startHour = balboa.filters.cycles[idx].startHour;
  *startMinute = balboa.filters.cycles[idx].startMinute;
  CalcStartDurationToEnd(
    balboa.filters.cycles[idx].startHour, balboa.filters.cycles[idx].startMinute,
    balboa.filters.cycles[idx].durationHours, balboa.filters.cycles[idx].durationMinutes,
    endHour, endMinute);
}

#if 0
0 	Filter 1 Start: Hour 	0-23
1 	Filter 1 Start: Minute 	0-59
2 	Filter 1 Duration: Hours 	0-23
3 	Filter 1 Duration: Minutes 	0-59
4 	Filter 2 Enable/Start: Hour 	Bits 0-6: Hour (0-23), Bit 7: Enable (0=OFF, 1=ON)
5 	Filter 2 Start: Minute 	0-59
6 	Filter 2 Duration: Hours 	0-23
7 	Filter 2 Duration: Minutes 	0-59
#endif
uint8_t ParseFilterCyclesMessage(uint8_t channel, const uint8_t *data, size_t nofBytes, BalboaFilterCycles_t *status) {
  if (nofBytes<8) { /* this is how many bytes we will read */
    McuLog_error("Filter Cycles message too short: %d", nofBytes);
    return ERR_FAILED;
  }
  status->cycles[0].isEnabled = true;
  status->cycles[0].startHour = data[0];
  status->cycles[0].startMinute = data[1];
  status->cycles[0].durationHours = data[2];
  status->cycles[0].durationMinutes = data[3];
  status->cycles[1].isEnabled = (data[4]&(1<<7))!=0;
  status->cycles[1].startHour = data[4]&0x7F;
  status->cycles[1].startMinute = data[5];
  status->cycles[1].durationHours = data[6];
  status->cycles[1].durationMinutes = data[7];
  status->isValid = true;
  return ERR_OK;
}

static uint8_t Balboa_QueueEnableDisableFilterCycle(uint8_t cycleNumber, bool onOff) {
  if (!balboa.filters.isValid) {
    return ERR_FAILED;
  }
  if (cycleNumber!=2) { /* can only turn on/off cycle 2 */
    McuLog_error("can only turn on or off filter cycle 2");
    return ERR_RANGE;
  }
  BalboaFilterCycles_t info = balboa.filters; /* struct copy of current values */
  info.cycles[cycleNumber-1].isEnabled = onOff; /* change value */
  Balboa_QueueRequestMessage(Request_Item_FilterCycles);
  Balboa_QueueSetFilterMessage(&info);
  Balboa_QueueRequestMessage(Request_Item_FilterCycles);
  return ERR_OK;
}

void Balboa_SetFilterCycle2OnOff(bool on) {
  (void)Balboa_QueueEnableDisableFilterCycle(2, on);
}

static uint8_t Balboa_QueueSetFilterCycleStartTime(uint8_t cycleNumber, uint8_t hour, uint8_t minute) {
  if (!balboa.filters.isValid) {
    return ERR_FAILED;
  }
  if (cycleNumber<1 || cycleNumber>2) { /* must be 1 or 2 */
    return ERR_RANGE;
  }
  BalboaFilterCycles_t info = balboa.filters; /* struct copy of current values */
  info.cycles[cycleNumber-1].startHour = hour; /* change value */
  info.cycles[cycleNumber-1].startMinute = minute; /* change value */
  Balboa_QueueRequestMessage(Request_Item_FilterCycles);
  Balboa_QueueSetFilterMessage(&info);
  Balboa_QueueRequestMessage(Request_Item_FilterCycles);
  return ERR_OK;
}

static uint8_t Balboa_QueueSetTemperatureRequest(float temperatureCelsius) {
  message_t msg;

  msg.kind = Message_Kind_SetTemperature;
  msg.u.setTemperature.valueCelcisus = temperatureCelsius;
  Balboa_QueueMessage(&msg);
  return ERR_OK;
}

void Balboa_SetTemperature(float temperature) {
  (void)Balboa_QueueSetTemperatureRequest(temperature);
}

void Balboa_SetFilterCycle1StartTime(uint8_t hour, uint8_t minute) {
  (void)Balboa_QueueSetFilterCycleStartTime(1, hour, minute);
}

void Balboa_SetFilterCycle2StartTime(uint8_t hour, uint8_t minute) {
  (void)Balboa_QueueSetFilterCycleStartTime(2, hour, minute);
}

static uint8_t Balboa_QueueSetFilterCycleEndTime(uint8_t cycleNumber, uint8_t hour, uint8_t minute) {
  if (!balboa.filters.isValid) {
    return ERR_FAILED;
  }
  if (cycleNumber<1 || cycleNumber>2) { /* must be 1 or 2 */
    return ERR_RANGE;
  }
  BalboaFilterCycles_t info = balboa.filters; /* struct copy of current values */
  /* calculate duration */
  uint32_t startMinute = info.cycles[cycleNumber-1].startHour*60 + info.cycles[cycleNumber-1].startMinute;
  uint32_t endMinute = hour*60 + minute;
  uint32_t deltaMinutes;

  if (startMinute<=endMinute) { /* no midnight in-between */
    deltaMinutes = endMinute-startMinute;
  } else { /* have midnight in-between */
    deltaMinutes = 24*60-startMinute + endMinute;
  }
  info.cycles[cycleNumber-1].durationHours = deltaMinutes/60; /* change value */
  info.cycles[cycleNumber-1].durationMinutes = deltaMinutes%60; /* change value */
  Balboa_QueueRequestMessage(Request_Item_FilterCycles);
  Balboa_QueueSetFilterMessage(&info);
  Balboa_QueueRequestMessage(Request_Item_FilterCycles);
  return ERR_OK;
}

void Balboa_SetFilterCycle1EndTime(uint8_t hour, uint8_t minute) {
  (void)Balboa_QueueSetFilterCycleEndTime(1, hour, minute);
}

void Balboa_SetFilterCycle2EndTime(uint8_t hour, uint8_t minute) {
  (void)Balboa_QueueSetFilterCycleEndTime(2, hour, minute);
}

static uint8_t checkAndUpdateBalboaTime(void) {
  /* Periodic check to see if the balboa controller time is set.
   * If time is not set, correct it.
   */
  TIMEREC time;
  DATEREC date;

  if (balboa.nofMessagesReceived<BALBOA_NOF_MIN_MESSAGES) {
    return ERR_BUSY; /* not enough messages received */
  }
  if (McuTimeDate_GetTimeDateAdjustDST(&time, &date)!=ERR_OK) {
    return ERR_FAILED;
  }
  if (date.Year<2025) { /* date/time has not been updated or is not valid? */
    return ERR_BUSY;
  }
  if (balboa.statusMsg.hour!=time.Hour || balboa.statusMsg.minute != time.Min) {
    McuLog_info("Updating balboa controller time from %02d:%02d to %02d:%02d", balboa.statusMsg.hour, balboa.statusMsg.minute, time.Hour, time.Min);
    Balboa_QueueSetTimeMessage(time.Hour, time.Min);
  }
  return ERR_OK;
}

static uint8_t ParseStatusUpdate(uint8_t channel, const uint8_t *data, size_t nofBytes, BalboaStatus_t *status) {
  /* decode status update message. Data points *after* the status update message type (0x13) */
  if (nofBytes<25) { /* this is how many bytes we will read */
    McuLog_error("Status update message too short: %d", nofBytes);
    return ERR_FAILED;
  }
  status->spaState = data[0]; /* 0x00=Running, 0x01=Initializing, 0x05=Hold Mode, ?0x14=A/B Temps ON?, 0x17=Test Mode */
  status->initializationMode = data[1]; /* 0x00=Idle, 0x01=Priming Mode, 0x02=?Fault?, 0x03=Reminder, 0x04=?Stage 1?, 0x05=?Stage 3?, 0x42=?Stage 2? */
  status->currentTemperature = data[2]; /* deci-Celsius, 0xFF if unknown */
  status->hour = data[3]; /* 0-23 */
  status->minute = data[4]; /* 0-59 */
  status->heatingMode = data[5]; /* 0=Ready, 1=Rest, 3=Ready-in-Rest */
  status->reminderType = data[6]; /* 0x00=None, 0x04=Clean filter, 0x0A=Check the pH, 0x09=Check the sanitizer, 0x1E=?Fault? */
  status->sensorAtemperatureHoldTimer = data[7]; /* Minutes if Hold Mode else Temperature (scaled by Temperature Scale) if A/B Temps else 0x01 if Test Mode else 0x00 */
  status->sensorBTemperature = data[8]; /* deci-Celsius temperature if A/B Temps else 0x00 */
  status->flags9 = data[9];
  status->flags10 = data[10];
  status->flags11 = data[11];
  status->flags12 = data[12];
  status->flags13 = data[13];
  status->flags14 = data[14];
  status->mister = data[15]!=0; /* 0=OFF, 1=ON */
  status->flags18 = data[18];
  status->flags19 = data[19];
  status->setTemperature = data[20]; /* deci-Celsius, 0xFF if unknown */
  status->flags21 = data[21];
  status->m8CylceTime = data[24]; /* 0=OFF; 30, 60, 90, or 120 (in minutes) */
  
  /* set values based on flags: */
  status->isCelsius = status->flags9&1;
  status->isLEDOn = (balboa.statusMsg.flags14&3)==3; /* zero is off */
  status->isPump1On = (balboa.statusMsg.flags11&3)!=0; /* zero is off */
  status->isPump2On = ((balboa.statusMsg.flags11>>2)&3)!=0; /* zero is off */
  status->isPumpCirculationOn = ((balboa.statusMsg.flags13>>1)&1)!=0; /* zero is off*/
  status->isLowTemperatureRange = (balboa.statusMsg.flags10&(1<<2))==0;
  return ERR_OK;
}

static void PrintMessageBytes(uint8_t channel, const uint8_t *data, size_t nofBytes) {
  unsigned char buf[128];

  McuUtility_strcpy(buf, sizeof(buf), (const unsigned char*)"0x");
  for (size_t i=0; i<nofBytes; i++) {
    McuUtility_strcatNum8Hex(buf, sizeof(buf), data[i]);
    if (i<nofBytes-1) {
      McuUtility_strcat(buf, sizeof(buf), (const unsigned char*)", 0x");
    }
  }
  McuLog_trace("ch:%02X message {%s}", channel, buf);
}

static uint8_t parsePacket(const uint8_t *packet, size_t packetSize) {
  /* data is including the framing byte at the beginning and end */
  uint8_t crc, len, channel, type;

  if (packet[0]!=0x7E) { /* check start delimiter */
    McuLog_error("Start delimiter error: %02X", packet[0]);
    return ERR_FRAMING;
  }
  if (packet[packetSize-1]!=0x7E) { /* check end delimiter */
    McuLog_error("End delimiter error: %02X", packet[packetSize-1]);
    return ERR_FRAMING;
  }
  len = packet[1]; /* check length */
  if (len<5 || len!=packetSize-2) {
    McuLog_error("Length error: %02X", len);
    return ERR_FAULT;
  }
  crc = calculate_crc(&packet[1], packetSize-3);
  if (crc!=packet[packetSize-2]) { /* check CRC */
    McuLog_error("CRC error: %02X, expected %02X", crc, packet[packetSize-2]);
    return ERR_CRC;
  }
  /* check unknown Byte 3: 0xAF when Byte 2 (channel) is 0xFF; 0xBF otherwise */
  channel = packet[2];
  if ((channel==0xff && packet[3]!=0xAF) || (channel!=0xff && packet[3]!=0xBF)) { /* check byte 3 */
    McuLog_error("Byte 3 error: %02X, expected %02X", packet[3], (channel==0xff)?0xAF:0xBF);
    return ERR_FAILED;
  }
  type = packet[4];
  //PrintMessageBytes(channel, packet, packetSize);
  switch(type) { /* check type */
    case 0x00: /* new client clear to send */
      if (Balboa_GetChannel()==0xFF) { /* no assigned channel yet */
        //PrintMessageBytes(channel, packet, packetSize);
        McuLog_info("ch:%02X New Client Clear to send, sending channel assignment request", channel);
        SendChannelAssignmentRequest();
      }
      return ERR_OK;
    case 0x01: /* channel assignment request */
      PrintMessageBytes(channel, packet, packetSize);
      McuLog_info("ch:%02X Channel Assignment request, type:%02x, hash:%02x, %02x", channel, packet[5], packet[6], packet[7]);
      return ERR_OK;
    case 0x02: /* channel assignment response */
      if (Balboa_GetChannel()==0xFF) { /* no assigned channel yet */
        PrintMessageBytes(channel, packet, packetSize);
        McuLog_info("ch:%02X Channel Assignment response -> ACK, assigned channel:%02x, hash:%02x, %02x", channel, packet[5], packet[6], packet[7]);
        /* todo: check for hash values */
        Balboa_SetChannel(packet[5]);
        SendChannelAssignementAcknowledge(Balboa_GetChannel());
      }
      return ERR_OK;
    case 0x03: /* channel assignment acknowledge */
      PrintMessageBytes(channel, packet, packetSize);
      McuLog_info("ch:%02X Channel assignment acknowledge");
      return ERR_OK;
    case 0x04: /* existing client request */
      if (channel==Balboa_GetChannel()) {
        PrintMessageBytes(channel, packet, packetSize);
        McuLog_info("ch:%02X Existing Client request");
        SendExistingClientResponse();
      }
      return ERR_OK;
    case 0x05: /* existing client response */
      PrintMessageBytes(channel, packet, packetSize);
      McuLog_info("ch:%02X Existing Client response");
      return ERR_OK;
    case 0x06: /* clear to send */
      //McuLog_info("ch:%02X Clear to send", channel);
      if (channel==Balboa_GetChannel()) {
        balboa.nofMessagesReceived++;
        Leds_Neg(LEDS_BLUE);
        //PrintMessageBytes(channel, packet, packetSize);
        if (Balboa_SendQueuedMessage()) {
          //McuLog_info("ch:%02X Clear to send -> message sent", channel);
        } else {
          //McuLog_info("ch:%02X Clear to send -> nothing to send", channel);
          SendNothingToSend(channel);
        }
      }
      return ERR_OK;
    case 0x07: /* nothing to send */
      //McuLog_info("ch:%02X Nothing to send", channel);
      return ERR_OK;
    case 0x13: /* status update */
      BalboaStatus_t status;
      uint8_t res;

      res = ParseStatusUpdate(channel, &packet[5], packetSize-5, &status);
      if (res==ERR_OK) {
        balboa.statusMsg = status; /* struct copy, update global status */
      }
      balboa.nofMessagesReceived++;
      Leds_Neg(LEDS_BLUE);
      return res;
    
    case 0x20: /* set temperature request */
      /* {0x7E, 0x06, 0x10, 0xBF, 0x20, 0x46, 0x3C, 0x7E} 0x47->70->35.0 C */
      PrintMessageBytes(channel, packet, packetSize);
      unsigned char buf[32];

      buf[0] = '\0';
      AppendTemperatureString(balboa.statusMsg.isCelsius, packet[5], buf, sizeof(buf));
      McuLog_info("ch:%02X Set temperature request: %02d %s", channel, packet[5], buf);
      return ERR_OK;
    case 0x21: /* set time request */
      PrintMessageBytes(channel, packet, packetSize);
      McuLog_info("ch:%02X Set time request: %02d:%02d", channel, packet[5], packet[6]);
      return ERR_OK;
    case 0x23: /* filter cycles message (response) */
      //PrintMessageBytes(channel, packet, packetSize);
      //McuLog_info("ch:%02X filter cycles message response", channel);
      BalboaFilterCycles_t filters;
      res = ParseFilterCyclesMessage(channel, &packet[5], packetSize-5, &filters);
      if (res==ERR_OK) {
        balboa.filters = filters; /* struct copy, update global status */
      }
      break;
    default:
      PrintMessageBytes(channel, packet, packetSize);
      return ERR_OK;
  }
  return ERR_OK;
}

static uint8_t PrintStatus(const McuShell_StdIOType *io) {
  unsigned char buf[96];

  McuShell_SendStatusStr((unsigned char*)"balboa", (const unsigned char*)"Balboa SPA controller status\r\n", io->stdOut);
  McuShell_SendStatusStr((unsigned char*)"  on", balboa.on?(const unsigned char*)"yes\r\n":(const unsigned char*)"no\r\n", io->stdOut);
  McuUtility_strcpy(buf, sizeof(buf), (unsigned char*)"0x");
  McuUtility_strcatNum8Hex(buf, sizeof(buf), Balboa_GetChannel());
  McuUtility_strcat(buf, sizeof(buf), (unsigned char*)"\r\n");
  McuShell_SendStatusStr((unsigned char*)"  channel", buf, io->stdOut);
  if (balboa.nofMessagesReceived<BALBOA_NOF_MIN_MESSAGES) {
    McuShell_SendStatusStr((unsigned char*)"  mode", (const unsigned char*)"not enough messages received\r\n", io->stdOut);
    return ERR_OK;
  }
  McuUtility_Num16uToStrFormatted(buf, sizeof(buf), balboa.statusMsg.hour, '0', 2);
  McuUtility_chcat(buf, sizeof(buf), ':');
  McuUtility_strcatNum16uFormatted(buf, sizeof(buf), balboa.statusMsg.minute, '0', 2);
  McuUtility_strcat(buf, sizeof(buf), (const unsigned char*)"\r\n");
  McuShell_SendStatusStr((unsigned char*)"  time", (const unsigned char*)buf, io->stdOut);
  McuShell_SendStatusStr((unsigned char*)"  units", balboa.statusMsg.isCelsius?(const unsigned char*)"Celsius\r\n":(const unsigned char*)"Fahrenheit\r\n", io->stdOut);
  McuShell_SendStatusStr((unsigned char*)"  led", balboa.statusMsg.isLEDOn?(const unsigned char*)"on\r\n":(const unsigned char*)"off\r\n", io->stdOut);
  McuShell_SendStatusStr((unsigned char*)"  pump 1", balboa.statusMsg.isPump1On?(const unsigned char*)"on\r\n":(const unsigned char*)"off\r\n", io->stdOut);
  McuShell_SendStatusStr((unsigned char*)"  pump 2", balboa.statusMsg.isPump2On?(const unsigned char*)"on\r\n":(const unsigned char*)"off\r\n", io->stdOut);
  McuShell_SendStatusStr((unsigned char*)"  circulation", balboa.statusMsg.isPumpCirculationOn?(const unsigned char*)"on\r\n":(const unsigned char*)"off\r\n", io->stdOut);
  McuShell_SendStatusStr((unsigned char*)"  low-temp", balboa.statusMsg.isLowTemperatureRange?(const unsigned char*)"yes (10-37C)\r\n":(const unsigned char*)"no (26.5-40C)\r\n", io->stdOut);
  McuUtility_strcpy(buf, sizeof(buf), GetHeatingModeString(balboa.statusMsg.heatingMode));
  McuUtility_strcat(buf, sizeof(buf), (const unsigned char*)"\r\n");
  McuShell_SendStatusStr((unsigned char*)"  heating", (const unsigned char*)buf, io->stdOut);

  /* filter cycles */
  if (balboa.filters.isValid) {
    unsigned char status[16];

    for(int i=0; i<sizeof(balboa.filters.cycles)/sizeof(balboa.filters.cycles[0]); i++) {
      McuUtility_strcpy(buf, sizeof(buf), balboa.filters.cycles[i].isEnabled? (const unsigned char*)"on, ":(const unsigned char*)"off, ");
      McuUtility_strcatNum16uFormatted(buf, sizeof(buf), balboa.filters.cycles[i].startHour, '0', 2);
      McuUtility_chcat(buf, sizeof(buf), ':');
      McuUtility_strcatNum16uFormatted(buf, sizeof(buf), balboa.filters.cycles[i].startMinute, '0', 2);
      McuUtility_strcat(buf, sizeof(buf), (const unsigned char*)", duration ");
      McuUtility_strcatNum16uFormatted(buf, sizeof(buf), balboa.filters.cycles[i].durationHours, '0', 2);
      McuUtility_chcat(buf, sizeof(buf), ':');
      McuUtility_strcatNum16uFormatted(buf, sizeof(buf), balboa.filters.cycles[i].durationMinutes, '0', 2);

      uint8_t endHour, endMinute;
      CalcStartDurationToEnd(
        balboa.filters.cycles[i].startHour, balboa.filters.cycles[i].startMinute,
        balboa.filters.cycles[i].durationHours, balboa.filters.cycles[i].durationMinutes,
        &endHour, &endMinute
      );
      McuUtility_strcat(buf, sizeof(buf), (const unsigned char*)", end ");
      McuUtility_strcatNum16uFormatted(buf, sizeof(buf), endHour, '0', 2);
      McuUtility_chcat(buf, sizeof(buf), ':');
      McuUtility_strcatNum16uFormatted(buf, sizeof(buf), endMinute, '0', 2);

      McuUtility_strcat(buf, sizeof(buf), (const unsigned char*)"\r\n");
      McuUtility_strcpy(status, sizeof(status), (unsigned char*)"  filter ");
      McuUtility_strcatNum8u(status, sizeof(status), i+1);
      McuShell_SendStatusStr(status, buf, io->stdOut);
    }
  } else {
    McuShell_SendStatusStr((unsigned char*)"  filter 1/2", (const unsigned char*)"not valid, run `balboa get cycle' first\r\n", io->stdOut);
  }

  McuUtility_Num32uToStr(buf, sizeof(buf), balboa.nofMessagesReceived);
  McuUtility_strcat(buf, sizeof(buf), (const unsigned char*)" messages received, spaState:");
  McuUtility_strcat(buf, sizeof(buf), GetSpaStateString(balboa.statusMsg.spaState));
  McuUtility_strcat(buf, sizeof(buf), (const unsigned char*)"\r\n");
  McuShell_SendStatusStr((unsigned char*)"  status", buf, io->stdOut);

  McuUtility_strcpy(buf, sizeof(buf), (const unsigned char*)"initMode:");
  McuUtility_strcat(buf, sizeof(buf), GetInitModeString(balboa.statusMsg.initializationMode));
  McuUtility_strcat(buf, sizeof(buf), (const unsigned char*)", reminder:");
  McuUtility_strcat(buf, sizeof(buf), GetReminderTypeString(balboa.statusMsg.reminderType));
  McuUtility_strcat(buf, sizeof(buf), (const unsigned char*)"\r\n");
  McuShell_SendStatusStr((unsigned char*)"  mode", buf, io->stdOut);

  buf[0] = '\0';
  McuUtility_strcat(buf, sizeof(buf), (const unsigned char*)"curr:");
  AppendTemperatureString(balboa.statusMsg.isCelsius, balboa.statusMsg.currentTemperature, buf, sizeof(buf));
  McuUtility_strcat(buf, sizeof(buf), (const unsigned char*)", set:");
  AppendTemperatureString(balboa.statusMsg.isCelsius, balboa.statusMsg.setTemperature, buf, sizeof(buf));
  if (balboa.statusMsg.spaState==0x05) { /* hold mode */
    McuUtility_strcat(buf, sizeof(buf), (const unsigned char*)" holdTimer:");
    McuUtility_strcatNum8u(buf, sizeof(buf), balboa.statusMsg.sensorAtemperatureHoldTimer);
    McuUtility_strcat(buf, sizeof(buf), (const unsigned char*)" min");
  } else if (balboa.statusMsg.flags21&(1<<1)) { /* A/B temps */
    McuUtility_strcat(buf, sizeof(buf), (const unsigned char*)", A:");
    AppendTemperatureString(balboa.statusMsg.isCelsius, balboa.statusMsg.sensorAtemperatureHoldTimer, buf, sizeof(buf));
  } else if (balboa.statusMsg.spaState==0x17 && balboa.statusMsg.sensorAtemperatureHoldTimer==0x01) { /* if test mode */
    McuUtility_strcat(buf, sizeof(buf), (const unsigned char*)", testMode:0x01");
  } else if (balboa.statusMsg.sensorAtemperatureHoldTimer==0) { /* must be zero otherwise? */
    McuUtility_strcat(buf, sizeof(buf), (const unsigned char*)", value:0x");
    McuUtility_strcatNum8Hex(buf, sizeof(buf), balboa.statusMsg.sensorAtemperatureHoldTimer);
  } else { /* show it as temperature? */
    McuUtility_strcat(buf, sizeof(buf), (const unsigned char*)", A:");
    AppendTemperatureString(balboa.statusMsg.isCelsius, balboa.statusMsg.sensorAtemperatureHoldTimer, buf, sizeof(buf));
  }
  McuUtility_strcat(buf, sizeof(buf), (const unsigned char*)", B:(0x");
  McuUtility_strcatNum8Hex(buf, sizeof(buf), balboa.statusMsg.sensorBTemperature);
  McuUtility_strcat(buf, sizeof(buf), (const unsigned char*)")??");
  AppendTemperatureString(balboa.statusMsg.isCelsius, balboa.statusMsg.sensorBTemperature, buf, sizeof(buf));
  McuUtility_strcat(buf, sizeof(buf), (const unsigned char*)"\r\n");
  McuShell_SendStatusStr((unsigned char*)"  temperature", buf, io->stdOut);

  McuShell_SendStatusStr((unsigned char*)"  mister", balboa.statusMsg.mister?(const unsigned char*)"on\r\n":(const unsigned char*)"off\r\n", io->stdOut);

  /* flags 9 */
  McuUtility_strcpy(buf, sizeof(buf), (const unsigned char*)"flags:0x");
  McuUtility_strcatNum8Hex(buf, sizeof(buf), balboa.statusMsg.flags9);
  if (balboa.statusMsg.flags9&(1<<0)) {
    McuUtility_strcat(buf, sizeof(buf), (const unsigned char*)", Celsius");
  } else {
    McuUtility_strcat(buf, sizeof(buf), (const unsigned char*)", Fahrenheit");
  }
  if (balboa.statusMsg.flags9&(1<<1)) {
    McuUtility_strcat(buf, sizeof(buf), (const unsigned char*)", 24-hour");
  } else {
    McuUtility_strcat(buf, sizeof(buf), (const unsigned char*)", 12-hour");
  }
  switch((balboa.statusMsg.flags9>>2)&3) { /* filter mode */
    case 0: McuUtility_strcat(buf, sizeof(buf), (const unsigned char*)", no filter cycle"); break;
    case 1: McuUtility_strcat(buf, sizeof(buf), (const unsigned char*)", filter cycle 1"); break;
    case 2: McuUtility_strcat(buf, sizeof(buf), (const unsigned char*)", filter cycle 2"); break;
    case 3: McuUtility_strcat(buf, sizeof(buf), (const unsigned char*)", filter cycle 1+2"); break;
    default: break;
  }
  if (balboa.statusMsg.flags9&(1<<5)) {
    McuUtility_strcat(buf, sizeof(buf), (const unsigned char*)", panel locked");
  } else {
    McuUtility_strcat(buf, sizeof(buf), (const unsigned char*)", panel not locked");
  }
  McuUtility_strcat(buf, sizeof(buf), (const unsigned char*)"\r\n");
  McuShell_SendStatusStr((unsigned char*)"  flags9", buf, io->stdOut);

  /* flags 10 */
  McuUtility_strcpy(buf, sizeof(buf), (const unsigned char*)"flags:0x");
  McuUtility_strcatNum8Hex(buf, sizeof(buf), balboa.statusMsg.flags10);
  if (balboa.statusMsg.flags10&(1<<2)) {
    McuUtility_strcat(buf, sizeof(buf), (const unsigned char*)", TemperatureRange:High");
  } else {
    McuUtility_strcat(buf, sizeof(buf), (const unsigned char*)", TemperatureRange:Low");
  }
  if (balboa.statusMsg.flags10&(1<<3)) {
    McuUtility_strcat(buf, sizeof(buf), (const unsigned char*)", ?needsHeat?:yes?");
  } else {
    McuUtility_strcat(buf, sizeof(buf), (const unsigned char*)", ?needsHeat?:no?");
  }
  switch((balboa.statusMsg.flags10>>4)&3) { /* heating state */
    case 0: McuUtility_strcat(buf, sizeof(buf), (const unsigned char*)", heating=off"); break;
    case 1: McuUtility_strcat(buf, sizeof(buf), (const unsigned char*)", heating=on"); break;
    case 2: McuUtility_strcat(buf, sizeof(buf), (const unsigned char*)", heating=waiting"); break;
    case 3: McuUtility_strcat(buf, sizeof(buf), (const unsigned char*)", heating=???"); break;
    default: break;
  }
  McuUtility_strcat(buf, sizeof(buf), (const unsigned char*)"\r\n");
  McuShell_SendStatusStr((unsigned char*)"  heating", buf, io->stdOut);

  /* flags 11 && 12 && 13 */
  buf[0] = '\0';
  switch(balboa.statusMsg.flags11&3) { /* pump1 status */
    case 0: McuUtility_strcat(buf, sizeof(buf), (const unsigned char*)"pump1=off"); break;
    case 1: McuUtility_strcat(buf, sizeof(buf), (const unsigned char*)"pump1=low"); break;
    case 2: McuUtility_strcat(buf, sizeof(buf), (const unsigned char*)"pump1=high"); break;
    case 3: McuUtility_strcat(buf, sizeof(buf), (const unsigned char*)"pump1=???"); break;
    default: break;
  }
  switch((balboa.statusMsg.flags11>>2)&3) { /* pump2 status */
    case 0: McuUtility_strcat(buf, sizeof(buf), (const unsigned char*)", pump2=off"); break;
    case 1: McuUtility_strcat(buf, sizeof(buf), (const unsigned char*)", pump2=low"); break;
    case 2: McuUtility_strcat(buf, sizeof(buf), (const unsigned char*)", pump2=high"); break;
    case 3: McuUtility_strcat(buf, sizeof(buf), (const unsigned char*)", pump2=???"); break;
    default: break;
  }
  switch((balboa.statusMsg.flags11>>4)&3) { /* pump3 status */
    case 0: McuUtility_strcat(buf, sizeof(buf), (const unsigned char*)", pump3=off"); break;
    case 1: McuUtility_strcat(buf, sizeof(buf), (const unsigned char*)", pump3=low"); break;
    case 2: McuUtility_strcat(buf, sizeof(buf), (const unsigned char*)", pump3=high"); break;
    case 3: McuUtility_strcat(buf, sizeof(buf), (const unsigned char*)", pump3=???"); break;
    default: break;
  }
  switch((balboa.statusMsg.flags11>>6)&3) { /* pump4 status */
    case 0: McuUtility_strcat(buf, sizeof(buf), (const unsigned char*)", pump4=off"); break;
    case 1: McuUtility_strcat(buf, sizeof(buf), (const unsigned char*)", pump4=low"); break;
    case 2: McuUtility_strcat(buf, sizeof(buf), (const unsigned char*)", pump4=high"); break;
    case 3: McuUtility_strcat(buf, sizeof(buf), (const unsigned char*)", pump4=???"); break;
    default: break;
  }
  switch(balboa.statusMsg.flags12&3) { /* pump5 status */
    case 0: McuUtility_strcat(buf, sizeof(buf), (const unsigned char*)", pump5=off"); break;
    case 1: McuUtility_strcat(buf, sizeof(buf), (const unsigned char*)", pump5=low"); break;
    case 2: McuUtility_strcat(buf, sizeof(buf), (const unsigned char*)", pump5=high"); break;
    case 3: McuUtility_strcat(buf, sizeof(buf), (const unsigned char*)", pump5=???"); break;
    default: break;
  }
  switch((balboa.statusMsg.flags12>>2)&3) { /* pump6 status */
    case 0: McuUtility_strcat(buf, sizeof(buf), (const unsigned char*)", pump6=off"); break;
    case 1: McuUtility_strcat(buf, sizeof(buf), (const unsigned char*)", pump6=low"); break;
    case 2: McuUtility_strcat(buf, sizeof(buf), (const unsigned char*)", pump6=high"); break;
    case 3: McuUtility_strcat(buf, sizeof(buf), (const unsigned char*)", pump6=???"); break;
    default: break;
  }
  switch((balboa.statusMsg.flags13>>1)&1) { /* circulation pump status */
    case 0: McuUtility_strcat(buf, sizeof(buf), (const unsigned char*)", circ=off"); break;
    case 1: McuUtility_strcat(buf, sizeof(buf), (const unsigned char*)", circ=on"); break;
    default: break;
  }
  switch((balboa.statusMsg.flags13>>2)&3) { /* blower status */
    case 0: McuUtility_strcat(buf, sizeof(buf), (const unsigned char*)", blower=off"); break;
    case 1: McuUtility_strcat(buf, sizeof(buf), (const unsigned char*)", blower=1?"); break;
    case 2: McuUtility_strcat(buf, sizeof(buf), (const unsigned char*)", blower=2?"); break;
    case 3: McuUtility_strcat(buf, sizeof(buf), (const unsigned char*)", blower=on"); break;
    default: break;
  }
  McuUtility_strcat(buf, sizeof(buf), (const unsigned char*)"\r\n");
  McuShell_SendStatusStr((unsigned char*)"  pumps", buf, io->stdOut);

  /* flags 14 */
  McuUtility_strcpy(buf, sizeof(buf), (const unsigned char*)"flags:0x");
  McuUtility_strcatNum8Hex(buf, sizeof(buf), balboa.statusMsg.flags14);
  switch(balboa.statusMsg.flags14&3) { /* light 1 status */
    case 0: McuUtility_strcat(buf, sizeof(buf), (const unsigned char*)", light1=off"); break;
    case 1: McuUtility_strcat(buf, sizeof(buf), (const unsigned char*)", light1=1?"); break;
    case 2: McuUtility_strcat(buf, sizeof(buf), (const unsigned char*)", light1=2?"); break;
    case 3: McuUtility_strcat(buf, sizeof(buf), (const unsigned char*)", light1=on"); break;
    default: break;
  }
  switch((balboa.statusMsg.flags14>>2)&3) { /* light 2 status */
    case 0: McuUtility_strcat(buf, sizeof(buf), (const unsigned char*)", light2=off"); break;
    case 1: McuUtility_strcat(buf, sizeof(buf), (const unsigned char*)", light2=1?"); break;
    case 2: McuUtility_strcat(buf, sizeof(buf), (const unsigned char*)", light2=2?"); break;
    case 3: McuUtility_strcat(buf, sizeof(buf), (const unsigned char*)", light2=on"); break;
    default: break;
  }
  McuUtility_strcat(buf, sizeof(buf), (const unsigned char*)"\r\n");
  McuShell_SendStatusStr((unsigned char*)"  lights", buf, io->stdOut);

  /* flags 21 */
  McuUtility_strcpy(buf, sizeof(buf), (const unsigned char*)"flags:0x");
  McuUtility_strcatNum8Hex(buf, sizeof(buf), balboa.statusMsg.flags21);
  if (balboa.statusMsg.flags21&(1<<1)) {
    McuUtility_strcat(buf, sizeof(buf), (const unsigned char*)", A/B Temperature:yes");
  } else {
    McuUtility_strcat(buf, sizeof(buf), (const unsigned char*)", A/B Temperature:no");
  }
  if (balboa.statusMsg.flags21&(1<<2)) {
    McuUtility_strcat(buf, sizeof(buf), (const unsigned char*)", timeouts=8 HR");
  } else {
    McuUtility_strcat(buf, sizeof(buf), (const unsigned char*)", timeouts=normal");
  }
  if (balboa.statusMsg.flags21&(1<<3)) {
    McuUtility_strcat(buf, sizeof(buf), (const unsigned char*)", settings locked=yes");
  } else {
    McuUtility_strcat(buf, sizeof(buf), (const unsigned char*)", settings locked=no");
  }
  McuUtility_strcat(buf, sizeof(buf), (const unsigned char*)"\r\n");
  McuShell_SendStatusStr((unsigned char*)"  flags21", buf, io->stdOut);

  McuUtility_Num8uToStr(buf, sizeof(buf), balboa.statusMsg.m8CylceTime);
  McuUtility_strcat(buf, sizeof(buf), (const unsigned char*)" minutes\r\n");
  McuShell_SendStatusStr((unsigned char*)"  M8 cylce", buf, io->stdOut);
  
  return ERR_OK;
}

static uint8_t PrintHelp(const McuShell_StdIOType *io) {
  McuShell_SendHelpStr((unsigned char*)"balboa", (unsigned char*)"Group of Balboa SPA controller commands\r\n", io->stdOut);
  McuShell_SendHelpStr((unsigned char*)"  help|status", (unsigned char*)"Show help or status\r\n", io->stdOut);
  McuShell_SendHelpStr((unsigned char*)"  on|off", (unsigned char*)"Set receiving messages on or off\r\n", io->stdOut);
  McuShell_SendHelpStr((unsigned char*)"  channel <ch>", (unsigned char*)"Set the communication channel. Use 0xff to request a new one.\r\n", io->stdOut);
  McuShell_SendHelpStr((unsigned char*)"  toggle pump1|pump2|light", (unsigned char*)"Toggle pumps or lights\r\n", io->stdOut);
  McuShell_SendHelpStr((unsigned char*)"  toggle range|heating", (unsigned char*)"Toggle temperature range (low/high) or heating mode (ready/rest)\r\n", io->stdOut);
  McuShell_SendHelpStr((unsigned char*)"  toggle reminder", (unsigned char*)"Toggle or clear last reminder\r\n", io->stdOut);
  McuShell_SendHelpStr((unsigned char*)"  time <hh:mm>", (unsigned char*)"Set time\r\n", io->stdOut);
  McuShell_SendHelpStr((unsigned char*)"  sync time", (unsigned char*)"Sync MCU time to balboa controller\r\n", io->stdOut);
  McuShell_SendHelpStr((unsigned char*)"  get cycle", (unsigned char*)"Request filter cylce information\r\n", io->stdOut);
  McuShell_SendHelpStr((unsigned char*)"  cycle 2 on|off", (unsigned char*)"Turn filter cycle 2 on or off\r\n", io->stdOut);
  McuShell_SendHelpStr((unsigned char*)"  cycle 1|2 start <hh:mm>", (unsigned char*)"Set filter cycle start time 1 or 2\r\n", io->stdOut);
  McuShell_SendHelpStr((unsigned char*)"  cycle 1|2 end <hh:mm>", (unsigned char*)"Set filter cycle end time 1 or 2\r\n", io->stdOut);
  McuShell_SendHelpStr((unsigned char*)"  temperature <temp>", (unsigned char*)"Set temperture in Celcius, e.g. 37.5, in 0.5 degree C steps\r\n", io->stdOut);
  return ERR_OK;
}

uint8_t Balboa_ParseCommand(const unsigned char* cmd, bool *handled, const McuShell_StdIOType *io) {
  const unsigned char *p;
  uint8_t hh, mm, ss, hs;
  int32_t number;
  int32_t channel;

  if (McuUtility_strcmp((char*)cmd, (char*)McuShell_CMD_HELP)==0 || McuUtility_strcmp((char*)cmd, (char*)"balboa help")==0) {
    *handled = true;
    return PrintHelp(io);
  } else if (McuUtility_strcmp((char*)cmd, (char*)McuShell_CMD_STATUS)==0 || McuUtility_strcmp((char*)cmd, (char*)"balboa status")==0) {
    *handled = true;
    return PrintStatus(io);
  } else if (McuUtility_strcmp((char*)cmd, (char*)"balboa on")==0) {
    *handled = true;
    return Balboa_SetIsOn(true);
  } else if (McuUtility_strcmp((char*)cmd, (char*)"balboa off")==0) {
    *handled = true;
    return Balboa_SetIsOn(false);
  } else if (McuUtility_strcmp((char*)cmd, (char*)"balboa toggle pump1")==0) {
    *handled = true;
    Balboa_QueueToggleMessage(Toggle_Item_Pump1);
    return ERR_OK;
  } else if (McuUtility_strcmp((char*)cmd, (char*)"balboa toggle pump2")==0) {
    *handled = true;
    Balboa_QueueToggleMessage(Toggle_Item_Pump2);
    return ERR_OK;
  } else if (McuUtility_strcmp((char*)cmd, (char*)"balboa toggle light")==0) {
    *handled = true;
    Balboa_QueueToggleMessage(Toggle_Item_Light1);
    return ERR_OK;
  } else if (McuUtility_strcmp((char*)cmd, (char*)"balboa toggle rang")==0) {
    *handled = true;
    Balboa_QueueToggleMessage(Toggle_Item_Temp_Range);
    return ERR_OK;
  } else if (McuUtility_strcmp((char*)cmd, (char*)"balboa toggle heating")==0) {
    *handled = true;
    Balboa_QueueToggleMessage(Toggle_Item_Heat_Mode);
    return ERR_OK;
    } else if (McuUtility_strcmp((char*)cmd, (char*)"balboa reminder")==0) {
    *handled = true;
    Balboa_QueueToggleMessage(Toggle_Item_Clear_Notification);
    return ERR_OK;
  } else if (McuUtility_strcmp((char*)cmd, (char*)"balboa get cycle")==0) {
    *handled = true;
    Balboa_QueueRequestMessage(Request_Item_FilterCycles);
    return ERR_OK;
  } else if (McuUtility_strncmp((char*)cmd, (char*)"balboa cycle ", sizeof("balboa cylce ")-1)==0) {
    p = cmd + sizeof("balboa cycle ")-1;
    *handled = true;
    if (McuUtility_xatoi(&p, &number)!=ERR_OK) {
      return ERR_FAILED;
    }
    if (number<1 || number>2) { /* number must be 1 or 2 */
      return ERR_FAILED;
    }
    if (McuUtility_strcmp(p, (char*)" on")==0) {
      if (number!=2) { /* can only turn on/off cycle 2 */
        return ERR_FAILED;
      }
      return Balboa_QueueEnableDisableFilterCycle(number, true);
    } else if (McuUtility_strcmp(p, (char*)" off")==0) {
      if (number!=2) { /* can only turn on/off cycle 2 */
        return ERR_FAILED;
      }
      return Balboa_QueueEnableDisableFilterCycle(number, false);
    } else if (McuUtility_strncmp(p, (char*)" start ", sizeof(" start ")-1)==0) {
      p += sizeof(" start ")-1;
      if (McuUtility_ScanTime(&p, &hh, &mm, &ss, &hs)!=ERR_OK) {
        return ERR_FAILED;
      }
      return Balboa_QueueSetFilterCycleStartTime(number, hh, mm);
    } else if (McuUtility_strncmp(p, (char*)" end ", sizeof(" end ")-1)==0) {
      p += sizeof(" end ")-1;
      if (McuUtility_ScanTime(&p, &hh, &mm, &ss, &hs)!=ERR_OK) {
        return ERR_FAILED;
      }
      return Balboa_QueueSetFilterCycleEndTime(number, hh, mm);
    }
    return ERR_FAILED;
  } else if (McuUtility_strncmp((char*)cmd, (char*)"balboa channel ", sizeof("balboa channel ")-1)==0) {
    p = cmd + sizeof("balboa channel ")-1;
    *handled = true;
    if (McuUtility_xatoi(&p, &channel)!=ERR_OK) {
      return ERR_FAILED;
    }
    Balboa_SetChannel(channel);
    return ERR_OK;
  } else if (McuUtility_strncmp((char*)cmd, (char*)"balboa time ", sizeof("balboa time ")-1)==0) {
    p = cmd + sizeof("balboa time ")-1;
    *handled = true;
    if (McuUtility_ScanTime(&p, &hh, &mm, &ss, &hs)!=ERR_OK) {
      return ERR_FAILED;
    }
    Balboa_SetTime(hh, mm);
    return ERR_OK;
  } else if (McuUtility_strcmp((char*)cmd, (char*)"balboa sync time")==0) {
    *handled = true;
    return checkAndUpdateBalboaTime();
  } else if (McuUtility_strncmp((char*)cmd, (char*)"balboa temperature ", sizeof("balboa temperature ")-1)==0) {
    p = cmd + sizeof("balboa temperature ")-1;
    *handled = true;
    int32_t integral;
    uint32_t fractional;
    uint8_t nofFractionalZeros;
    if (McuUtility_ScanDecimal32sDotNumber(&p, &integral, &fractional, &nofFractionalZeros)!=ERR_OK) {
      return ERR_FAILED;
    }
    if (integral<0) { /* only positive falues allowed */
      return ERR_FAILED;
    }
    if (fractional!=0 && nofFractionalZeros>0) { /* do not accept fractional zeros */
      return ERR_FAILED;
    }
    return Balboa_QueueSetTemperatureRequest((float)integral + (float)fractional*0.1f);
  }
  return ERR_OK;
}

static void getByte(uint8_t *p) {
  if (McuUart485_GetRxQueueByte(p, portMAX_DELAY)!=ERR_OK) {
    McuLog_fatal("failed rx data byte");
    for(;;) {
      /* failed */
    }
  }
}

static void balboaTask(void *pvParameters) {
  uint8_t packet[48];
  size_t i;
  uint8_t len, channel, type, crc;
  TickType_t lastTimeCheckTickCount, currTickCount;
  uint32_t deltaSinceLastCheck_ms;
#if PL_CONFIG_USE_WATCHDOG
  TickType_t tickCount;
#endif

#if PL_CONFIG_USE_MININI
  balboa.on = McuMinINI_ini_getbool(NVMC_MININI_SECTION_BALBOA, NVMC_MININI_KEY_BALBOA_ON, false, NVMC_MININI_FILE_NAME);
  balboa.assignedChannel = McuMinINI_ini_getl(NVMC_MININI_SECTION_BALBOA, NVMC_MININI_KEY_BALBOA_CHANNEL, 0x11, NVMC_MININI_FILE_NAME);
#else
  balboa.on = false;  /* default bus listening */
  balboa.assignedChannel = 0x11; //0xFF; /* not assigned yet */
#endif
  lastTimeCheckTickCount = xTaskGetTickCount();
  for(;;) {
    if (!balboa.on) {
      #if PL_CONFIG_USE_WATCHDOG
        McuWatchdog_DelayAndReport(McuWatchdog_REPORT_ID_TASK_BALBOA, 1, 100);
      #else
        vTaskDelay(pdMS_TO_TICKS(100));
      #endif
      continue;
    }

    /* The balboa controller has no battery backup for the clock/time: try to update it if it is not set */
    currTickCount = xTaskGetTickCount();
    deltaSinceLastCheck_ms = pdMS_TO_TICKS(currTickCount-lastTimeCheckTickCount);
    if (deltaSinceLastCheck_ms>(5*60*1000) /* every 5 minutes */
         || (    balboa.nofMessagesReceived>BALBOA_NOF_MIN_MESSAGES  /* or it just has been powered on? */
              && balboa.statusMsg.hour==12 /* power-on value of balboa is 12:00 */
              && balboa.statusMsg.minute==0
              && deltaSinceLastCheck_ms>(30*1000) /* wait at least 30 seconds */
            )
        ) 
    { 
      checkAndUpdateBalboaTime();
      lastTimeCheckTickCount = currTickCount;
    }

    i = 0;
  #if PL_CONFIG_USE_WATCHDOG
    /* measure tick count at the start of parsing. Will report it at the end of a message */
    tickCount = McuWatchdog_ReportTimeStart();
  #endif
    for(;;) { /* detect and parse a message, breaks && continues */
      if (i>=sizeof(packet)) {
        McuLog_error("packet too long: %d", i);
        break;
      }
      getByte(&packet[i]);
      if (i==0) {
        len = channel = type = crc = 0; /* init */
        if (packet[0]!=0x7E) { /* start of message */
          McuLog_error("wrong start of message:%02X", packet[i]);
          McuUart485_ClearRxQueue();
          break;
        }
        i++;
      } else if (i==1 && packet[1]==0x7E) { /* got 0x7E 0x7E: must be end of message and start of new one */
        McuLog_info("sync on start of message");
        continue; /* continue where we are */
      } else if (i==1) { /* length */
        len = packet[1];
        if (len<5) { /* minimum length is 5 */
          McuLog_error("length too small: %d", len);
          break;
        }
        i++;
      } else if (i==2) { /* channel */
        channel = packet[2];
        i++;
      } else if (i==3) { /* check special? byte in protocol */
        if ((packet[2]==0xff && packet[3]==0xAF) || (packet[2]!=0xff && packet[3]==0xBF)) {
          /* ok */
        } else  {
          McuLog_error("byte 3 error: %02X", packet[3]);
          break;
        }
        i++;
      } else if (i==4) { /* type */
        type = packet[4];
        i++;
      } else if (i<len) { /* read data */
        i++;
      } else if (i==len) {
        crc = packet[i];
        uint8_t crc2 = calculate_crc(&packet[1], len-1);
        if (crc!=crc2) { /* check CRC */
          McuLog_error("CRC error: %02X, expected %02X", crc, crc2);
          break;
        }
        i++;
      } else if (i==len+1) {
        if (packet[i]!=0x7E) { /* wrong end of message*/
          McuLog_error("wrong end of message: %02X", packet[i]);
          break;
        } else {
          parsePacket(packet, len+2); /* +2: adding start and end marker */
          break; /* restart */
        }
      } /* if */
    } /* for (detect and parse message) */
  #if PL_CONFIG_USE_WATCHDOG
    McuWatchdog_ReportTimeEnd(McuWatchdog_REPORT_ID_TASK_BALBOA, tickCount);
  #endif
    if (balboa.on && !balboa.filters.isValid && balboa.nofMessagesReceived>BALBOA_NOF_MIN_MESSAGES && (balboa.nofMessagesReceived%BALBOA_NOF_MIN_MESSAGES)==0) {
      Balboa_QueueRequestMessage(Request_Item_FilterCycles); /* send a filter cycle message shortly after startup */
    }
  } /* for (task loop) */
}

void Balboa_Deinit(void) {
  vQueueDelete(messageQueue);
  messageQueue = NULL;
}

void Balboa_Init(void) {
    if (xTaskCreate(
      balboaTask,  /* pointer to the task */
      "balboa", /* task name for kernel awareness debugging */
      (4*1024)/sizeof(StackType_t), /* task stack size */
      (void*)NULL, /* optional task startup argument */
      tskIDLE_PRIORITY+4,  /* initial priority */
      (TaskHandle_t*)NULL /* optional task handle to create */
    ) != pdPASS)
  {
    McuLog_fatal("Failed creating RS-485 task");
    for(;;){} /* error! probably out of memory */
  }
  messageQueue = xQueueCreate(MESSAGE_QUEUE_LENGTH, MESSAGE_QUEUE_ELEM_SIZE);
  if (messageQueue==NULL) {
    McuLog_fatal("failed creating messsage queue");
    for(;;){} /* out of memory? */
  }
  vQueueAddToRegistry(messageQueue, "messageQueue");
}

#endif /* PL_CONFIG_USE_BALBOA */
