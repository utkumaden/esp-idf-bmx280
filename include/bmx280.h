/**
 * BMX280 - BME280 & BMP280 Driver for Esspressif ESP-32.
 * Copyright (C) 2020
 * H. Utku Maden <utkumaden@hotmail.com>
 */
#ifndef _BMX280_H_
#define _BMX280_H_

#include <stdint.h>
#include "driver/i2c.h"
#include "sdkconfig.h"

/**
 * Anonymous structure to driver settings.
 */
typedef struct bmx280_t bmx280_t;

bmx280_t* bmx280_create();
void bmx280_close(bmx280_t* bmx280);

esp_err_t bmx280_init(bmx280_t* bmx280);
esp_err_t bmx280_configure(bmx280_t* bmx280);

esp_err_t bmx280_setCyclicMeasure(bmx280_t* bmx280);

esp_err_t bmx280_forceMeasurement(bmx280_t* bmx280);
esp_err_t bmx280_readOutFloat(bmx280_t* bmx280, float* temperature, float* pressure, float* humidity);

#endif
