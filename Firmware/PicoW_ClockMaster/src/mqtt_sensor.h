/*
 * Copyright (c) 2025, Erich Styger
 *
 * SPDX-License-Identifier: BSD-3-Clause
 */

#ifndef MQTT_SENSOR_H_
#define MQTT_SENSOR_H_

#ifdef __cplusplus
extern "C" {
#endif

#include <stdint.h>
#include <stdbool.h>
#include "mqtt_client.h"

typedef enum topic_ID_e { /* IDs for topics we can subscribe to */
  Topic_ID_None,
  Topic_ID_Sensor_Enable,
  Topic_ID_Sensor_Temperature,
  Topic_ID_Sensor_Humidity,
} topic_ID_e;

bool MqttSensor_GetIsEnabled(void);

int MqttSensor_Publish_Enabled(bool isEnabled);

int MqttSensor_Publish_SensorValues(float temperature, float humidity);

void MqttSensor_incoming_publish_cb(void *arg, const char *topic, uint32_t tot_len);

void MqttSensor_incoming_data_cb(void *arg, const uint8_t *data, uint16_t len, uint8_t flags);

void MqttSensor_connection_cb(mqtt_client_handle client, void *arg, int /*mqtt_connection_status_t*/ status);

#ifdef __cplusplus
}  /* extern "C" */
#endif

#endif /* MQTT_SENSOR_H_ */