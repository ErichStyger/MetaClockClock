/*
 * Copyright (c) 2025, Erich Styger
 *
 * SPDX-License-Identifier: BSD-3-Clause
 */

#include "platform.h"
#if PL_CONFIG_USE_MQTT_SENSOR
#if PL_CONFIG_USE_PICO_W
  #include "pico/cyw43_arch.h"
#endif
#include "lwip/apps/mqtt.h"
#include "mqtt_sensor_config.h"
#include "mqtt_client.h"
#include "mqtt_sensor.h"
#include "McuRTOS.h"
#include "McuUtility.h"
#include "McuWatchdog.h"
#include "McuLog.h"
#include "cJSON.h"

static bool SensorIsEnabled = true;

bool MqttSensor_GetIsEnabled(void) {
  return SensorIsEnabled;
}

void MqttSensor_SetEnabledIsOn(bool isOn) {
  SensorIsEnabled = isOn;
}

static void IncomingSwitch(const u8_t *data, u16_t len, const char *logMsg, void (setter)(bool)) {
  unsigned char buf[32];

  MqttClient_GetDataString(buf, sizeof(buf), data, len);
  if (mqtt_doLogging()) {
    McuLog_trace("Rx: %s: %s", logMsg, buf);
  }
  cJSON *json = cJSON_Parse((char*)buf);
  if (json==NULL) {
    McuLog_error("Failed to parse JSON: %s", buf);
    return;
  }
  const cJSON *state = cJSON_GetObjectItemCaseSensitive(json, "state");
  if (cJSON_IsString(state) && (state->valuestring != NULL))  {
    McuLog_info("Checking state \"%s\"", state->valuestring);
    if (McuUtility_strcmp((char*)state->valuestring, (char*)"ON")==0) {
      setter(true);
    } else if(McuUtility_strcmp((char*)state->valuestring, (char*)"OFF")==0) {
      setter(false);
    }
  }
  cJSON_Delete(json); /* free object */
}

void MqttSensor_incoming_data_cb(void *arg, const u8_t *data, u16_t len, u8_t flags) {
  LWIP_UNUSED_ARG(data);

#if MQTT_CLIENT_CONFIG_EXTRA_LOGS
  const struct mqtt_connect_client_info_t *client_info = (const struct mqtt_connect_client_info_t*)arg;
  McuLog_trace("MQTT client \"%s\" data cb: len %d, flags %d", client_info->client_id, (int)len, (int)flags);
#endif
  if(flags & MQTT_DATA_FLAG_LAST) {
    topic_ID_e id = mqtt_get_in_pub_ID();
    /* Last fragment of payload received (or whole part if payload fits receive buffer. See MQTT_VAR_HEADER_BUFFER_LEN)  */
    if (id == Topic_ID_Sensor_Temperature) {
      McuLog_trace("Temperature");
    } else if (id == Topic_ID_Sensor_Humidity) {
      McuLog_trace("Humidity");
    } else if (id == Topic_ID_Sensor_Enable) {
      McuLog_trace("Enable");
      IncomingSwitch(data, len, "sensor enable", MqttSensor_SetEnabledIsOn);
    } else {
      McuLog_trace("mqtt_incoming_data_cb, id: %d: Ignoring payload...", id);
    }
  } else {
    McuLog_trace("mqtt_incoming_data_cb: fragmented payload ...");
    /* Handle fragmented payload, store in buffer, write to file or whatever */
  }
}

void MqttSensor_incoming_publish_cb(void *arg, const char *topic, u32_t tot_len) {
  const struct mqtt_connect_client_info_t *client_info = (const struct mqtt_connect_client_info_t*)arg;
  if (McuUtility_strcmp(topic, MQTT_SENSOR_CONFIG_TOPIC_NAME_SENSOR_HUMIDITY)==0) {
    mqtt_set_in_pub_ID(Topic_ID_Sensor_Temperature);
  } else if (McuUtility_strcmp(topic, MQTT_SENSOR_CONFIG_TOPIC_NAME_SENSOR_HUMIDITY)==0) {
    mqtt_set_in_pub_ID(Topic_ID_Sensor_Humidity);
  } else if (McuUtility_strcmp(topic, MQTT_SENSOR_CONFIG_TOPIC_NAME_SENSOR_ENABLE)==0) {
    mqtt_set_in_pub_ID(Topic_ID_Sensor_Enable);
  } else { /* unknown */
    McuLog_trace("MQTT client \"%s\" incoming cb: topic %s, len %d", client_info->client_id, topic, (int)tot_len);
    mqtt_set_in_pub_ID(Topic_ID_None);
  }
}

void MqttSensor_connection_cb(mqtt_client_handle client, void *arg, int /*mqtt_connection_status_t*/ status) {
  const struct mqtt_connect_client_info_t *client_info = (const struct mqtt_connect_client_info_t*)arg;

  if (status == MQTT_CONNECT_ACCEPTED) {
    /* subscribe to topics */
  	mqtt_subscribeTopic(client, client_info, MQTT_SENSOR_CONFIG_TOPIC_NAME_SENSOR_ENABLE);
  }
}

static void mqtt_publish_request_cb(void *arg, err_t err) {
#if 0 && MQTT_CLIENT_CONFIG_EXTRA_LOGS /* be careful not to delay callback too much */
  const struct mqtt_connect_client_info_t *client_info = (const struct mqtt_connect_client_info_t*)arg;
  McuLog_trace("MQTT client \"%s\" publish request cb: err %d", client_info->client_id, (int)err);
#endif
}

static int publish(char *buf, const char *topic) {
  err_t res;
  const uint8_t qos = 0; /* quos: 0: fire&forget, 1: at least once */
  const uint8_t retain = 0;

  if (!MqttClient_CanPublish()) {
    return ERR_DISABLED;
  }
  res = mqtt_publish(mqtt_getClient(), topic, buf, strlen((char*)buf), qos, retain, mqtt_publish_request_cb, NULL);
  if (res!=ERR_OK) {
    McuLog_error("Failed topic %s mqtt_publish: %d", topic, res);
    (void)MqttClient_Disconnect(); /* try disconnect and connect again */
    (void)MqttClient_Connect();
    return ERR_FAILED;
  }
  if (mqtt_doLogging()) {
    McuLog_trace("published %s: %s", topic, buf);
  }
  return ERR_OK;
}

int MqttSensor_Publish_Enabled(bool isEnabled) {
  uint8_t buf[64];

  McuUtility_strcpy(buf, sizeof(buf), (unsigned char*)"{\"state\": ");
  if (isEnabled) {
    McuUtility_strcat(buf, sizeof(buf), (unsigned char*)" \"ON\"");
  } else {
    McuUtility_strcat(buf, sizeof(buf), (unsigned char*)" \"OFF\"");
  }
  McuUtility_strcat(buf, sizeof(buf), (unsigned char*)"}");
  return publish((char*)buf, (char*)MQTT_SENSOR_CONFIG_TOPIC_NAME_SENSOR_ENABLE);
}

int MqttSensor_Publish_SensorValues(float temperature, float humidity) {
  err_t res;
  uint8_t buf[64];


  McuUtility_strcpy(buf, sizeof(buf), (unsigned char*)"{\"temperature\": ");
  McuUtility_strcatNumFloat(buf, sizeof(buf), temperature, 2);
  McuUtility_strcat(buf, sizeof(buf), (unsigned char*)", \"unit\": \"°C\"}");
  res = publish((char*)buf, (char*)MQTT_SENSOR_CONFIG_TOPIC_NAME_SENSOR_TEMPERATURE);
  if (res!=ERR_OK) {
    return res;
  }

  McuUtility_strcpy(buf, sizeof(buf), (unsigned char*)"{\"humidity\": ");
  McuUtility_strcatNumFloat(buf, sizeof(buf), humidity, 2);
  McuUtility_strcat(buf, sizeof(buf), (unsigned char*)", \"unit\": \"%\"}");
  return publish((char*)buf, (char*)MQTT_SENSOR_CONFIG_TOPIC_NAME_SENSOR_HUMIDITY);
}

#endif /* PL_CONFIG_USE_MQTT_SENSOR */
