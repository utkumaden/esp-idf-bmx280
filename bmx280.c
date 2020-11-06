/**
 * BMX280 - BME280 & BMP280 Driver for Esspressif ESP-32.
 * Copyright (C) 2020
 * H. Utku Maden <utkumaden@hotmail.com>
 */
#include "bmx280.h"
#include "esp_log.h"

// [BME280] Register address of humidity least significant byte.
#define BMX280_REG_HUMI_LSB 0xFE
// [BME280] Register address of humidity most significant byte.
#define BMX280_REG_HUMI_MSB 0xFD

// Register address of temperature fraction significant byte.
#define BMX280_REG_TEMP_XSB 0xFC
// Register address of temperature least significant byte.
#define BMX280_REG_TEMP_LSB 0xFB
// Register address of temperature most significant byte.
#define BMX280_REG_TEMP_MSB 0xFA

// Register address of pressure fraction significant byte.
#define BMX280_REG_PRES_XSB 0xF9
// Register address of pressure least significant byte.
#define BMX280_REG_PRES_LSB 0xF8
// Register address of pressure most significant byte.
#define BMX280_REG_PRES_MSB 0xF7

// Register address of sensor configuration.
#define BMX280_REG_CONFIG 0xF5
// Register address of sensor measurement control.
#define BMX280_REG_MESCTL 0xF4
// Register address of sensor status.
#define BMX280_REG_STATUS 0xF3
// [BME280] Register address of humidity control.
#define BMX280_REG_HUMCTL 0xF2

// [BME280] Register address of calibration constants. (high bank)
#define BMX280_REG_CAL_HI 0xE1
// Register address of calibration constants. (low bank)
#define BMX280_REG_CAL_LO 0x88

// Register address for sensor reset.
#define BXM280_REG_RESET 0xE0
// Chip reset vector.
#define BMX280_RESET_VEC 0xB6

// Register address for chip identification number.
#define BMX280_REG_CHPID 0xD0
// Value of REG_CHPID for BME280
#define BME280_ID  0x60
// Value of REG_CHPID for BMP280 (Engineering Sample 1)
#define BMP280_ID0 0x56
// Value of REG_CHPID for BMP280 (Engineering Sample 2)
#define BMP280_ID1 0x57
// Value of REG_CHPID for BMP280 (Production)
#define BMP280_ID2 0x58

struct bmx280_t{
    i2c_port_t i2c_port;
    uint8_t slave;
    uint8_t chip_id;
};

/**
 * Macro that identifies a chip id as BME280 or BMP280
 * @note Only use when the chip is verified to be either a BME280 or BMP280.
 * @see bmx280_verify
 * @param chip_id The chip id.
 */
#define bmx280_isBME(chip_id) ((chip_id) == BME280_ID)
/**
 * Macro to verify a the chip id matches with the expected values.
 * @note Use when the chip needs to be verified as a BME280 or BME280.
 * @see bmx280_isBME
 * @param chip_id The chip id.
 */
#define bmx280_verify(chip_id) (((chip_id) == BME280_ID) || ((chip_id) == BMP280_ID2) || ((chip_id) == BMP280_ID1) || ((chip_id) == BMP280_ID0))

/**
 * Returns false if the sensor was not found.
 * @param bmx280 The driver structure.
 */
#define bmx280_validate(bmx280) (!(bmx280->slave == 0xDE && bmx280->chip_id == 0xAD))

/**
 * Read from sensor.
 * @param bmx280 Driver Sturcture.
 * @param addr Register address.
 * @param dout Data to read.
 * @param size The number of bytes to read.
 * @returns Error codes.
 */
static esp_err_t bmx280_read(bmx280_t *bmx280, uint8_t addr, uint8_t *dout, size_t size)
{
    esp_err_t err;
    i2c_cmd_handle_t cmd = i2c_cmd_link_create();
    if (cmd)
    {
        // Write register address
        i2c_master_start(cmd);
        i2c_master_write_byte(cmd, bmx280->slave | I2C_MASTER_READ, true);
        i2c_master_write_byte(cmd, addr, true);
        i2c_master_stop(cmd);

        // Read Registers
        i2c_master_start(cmd);
        i2c_master_read(cmd, dout, size, true);
        i2c_master_stop(cmd);

        err = i2c_master_cmd_begin(bmx280->i2c_port, cmd, 5);

        i2c_cmd_link_delete(cmd);
        return err;
    }
    else 
    {
        return ESP_ERR_NO_MEM; 
    }
}

static esp_err_t bmx280_write(bmx280_t* bmx280, uint8_t addr, const uint8_t *din, size_t size)
{
    esp_err_t err;
    i2c_cmd_handle_t cmd = i2c_cmd_link_create();
    if (cmd)
    {
        i2c_master_start(cmd);
        i2c_master_write_byte(cmd, bmx280->slave | I2C_MASTER_WRITE, true);
        for (int i = 0; i < size; i++)
        {
            // Register
            i2c_master_write_byte(cmd, addr + i, true);
            //Data
            i2c_master_write_byte(cmd, din[i], true);
        }
        i2c_master_stop(cmd);

        err = i2c_master_cmd_begin(bmx280->i2c_port, cmd, 5);
        i2c_cmd_link_delete(cmd);
        return err;
    }
    else
    {
        return ESP_ERR_NO_MEM;
    }
}

static esp_err_t bmx280_probe_address(bmx280_t *bmx280)
{
    esp_err_t err = bmx280_read(bmx280, BMX280_REG_CHPID, &bmx280->chip_id, sizeof bmx280->chip_id);

    if (err == ESP_OK)
    {
        if (
        #if CONFIG_BMX280_EXPECT_BME280
            bmx280->chip_id == BME280_ID
        #elif CONFIG_BMX280_EXPECT_BMP280
            bmx280->chip_id == BMP280_ID2 || bmx280->chip_id == BMP280_ID1 || bmx280->chip_id == BMP280_ID0
        #else
            bmx280_verify(bmx280->chip_id)
        #endif
        )
        {
            ESP_LOGI("bmx280", "Probe success: address=%hhu, id=%hhu", bmx280->slave, bmx280->chip_id);
           return ESP_OK;
        }
        else
        {
            err = ESP_ERR_NOT_FOUND;
        }
    }

    ESP_LOGW("bmx280", "Probe failure: address=%hhu, id=%hhu, reason=%s", bmx280->slave, bmx280->chip_id, esp_err_to_name(err));
    return err;
}

static esp_err_t bmx280_probe(bmx280_t *bmx280)
{
    ESP_LOGI("bmx280", "Probing for BMP280/BME280 sensors on I2C %d", bmx280->i2c_port);

    #if CONFIG_BMX280_ADDRESS_HI
    bmx280->slave = 0xEE;
    return bmx280_probe_address(bmx280);
    #elif CONFIG_BMX280_ADDRESS_LO
    bmx280->slave = 0xEC;
    return bmx280_probe_address(bmx280);
    #else
    esp_err_t err;
    bmx280->slave = 0xEC;
    if ((err = bmx280_probe_address(bmx280)) != ESP_OK)
    {
        bmx280->slave = 0xEE;
        if ((err = bmx280_probe_address(bmx280)) != ESP_OK)
        {
            ESP_LOGE("bmx280", "Sensor not found.");
            bmx280->slave = 0xDE;
            bmx280->chip_id = 0xAD;
        }
    }
    return err;
    #endif
}
