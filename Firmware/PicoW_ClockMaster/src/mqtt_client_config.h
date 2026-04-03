/*
 * Copyright (c) 2025, Erich Styger
 *
 * SPDX-License-Identifier: BSD-3-Clause
 */

#ifndef MQTT_CLIENT_CONFIG_H_
#define MQTT_CLIENT_CONFIG_H_

/* default entries for the MQTT broker connection: */
#ifndef MQTT_CLIENT_CONFIG_BROKER
  #define MQTT_CLIENT_CONFIG_BROKER   "broker"
    /*!< host name or IP of the MQTT broker */
#endif

#ifndef MQTT_CLIENT_CONFIG_CLIENT
  #define MQTT_CLIENT_CONFIG_CLIENT   "client"
    /*!< MQTT client ID: use a different client ID for each connection */
#endif

#ifndef MQTT_DEFAULT_USER
  #define MQTT_DEFAULT_USER           "user"
    /*!< user name for the MQTT connection */
#endif

#ifndef MQTT_CLIENT_CONFIG_PASS
  #define MQTT_CLIENT_CONFIG_PASS     "password"
    /*!< password for the MQTT connection */
#endif

#ifndef MQTT_CLIENT_CONFIG_PUBLISH
  #define MQTT_CLIENT_CONFIG_PUBLISH  (1)
    /*!< if set to 1, the client will publish messages */
#endif

#ifndef MQTT_CLIENT_CONFIG_EXTRA_LOGS
  #define MQTT_CLIENT_CONFIG_EXTRA_LOGS   (0)
    /*!< set to 1 to produce extra log output */
#endif

#ifndef MQTT_CLIENT_CONFIG_HEADER_FILE
  #define MQTT_CLIENT_CONFIG_HEADER_FILE            "my_header.h"
    /*!< header file to include for the MQTT client configuration */
#endif 

#ifndef MQTT_CLIENT_INCOMING_PUBLISH_CALLBACK
  #define MQTT_CLIENT_INCOMING_PUBLISH_CALLBACK     My_incoming_publish_cb
    /*! callback for incoming MQTT messages */
#endif

#ifndef MQTT_CLIENT_INCOMING_DATA_CALLBACK
  #define MQTT_CLIENT_INCOMING_DATA_CALLBACK        My_incoming_data_cb
    /*! callback for each fragment of payload that arrives */
#endif

#ifndef MQTT_CLIENT_CONNECTION_CALLBACK
  #define MQTT_CLIENT_CONNECTION_CALLBACK           My_connection_cb
    /*! callback for connection events */
#endif

#endif /* MQTT_CLIENT_CONFIG_H_ */
