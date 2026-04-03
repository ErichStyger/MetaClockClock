/*
 * Copyright (c) 2023, Erich Styger
 *
 * SPDX-License-Identifier: BSD-3-Clause
 */

#ifndef SENSOR_H_
#define SENSOR_H_

/*!
 * \brief Getter for the sensor temperature
 * \return Sensor temperature value in degree C
 */
float Sensor_GetTemperature(void);

/*!
 * \brief Getter for the sensor humidity
 * \return Sensor humidity value as % RH
 */
float Sensor_GetHumidity(void);

/*!
 * \brief Sensor module de-initialization
 */
void Sensor_Deinit(void);

/*!
 * \brief Sensor module initialization
 */
void Sensor_Init(void);

#endif /* SENSOR_H_ */
