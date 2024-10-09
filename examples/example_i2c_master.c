#include <esp_err.h>
#include <esp_log.h>
#include <driver/i2c_master.h>
#include "bmx280.h"

#define I2C_MASTER_SCL_IO 32      // GPIO number for I2C master clock
#define I2C_MASTER_SDA_IO 33      // GPIO number for I2C master data
#define I2C_MASTER_NUM 1          // I2C master i2c port number
#define I2C_MASTER_FREQ_HZ 100000 // I2C master clock frequency
#define BME280_ADDRESS 0x77

static const char *TAG = "Explore I2C master new lib";

void i2c_scanner(i2c_master_bus_handle_t bus_handle)
{
    printf("Scanning I2C bus...\n");
    for (uint8_t i = 1; i < 127; i++)
    {
        esp_err_t ret = i2c_master_probe(bus_handle, i, -1);
        if (ret == ESP_OK)
        {
            ESP_LOGI(TAG, "Found device on bus with address: %d", i);
        }
        else
        {
            ESP_LOGE(TAG, "No device found on address: %d", i);
        }
    }
    printf("Scan completed\n");
}

i2c_master_bus_handle_t config_i2c_bus()
{
    i2c_master_bus_config_t i2c_master_bus_config = {
        .clk_source = I2C_CLK_SRC_DEFAULT,
        .i2c_port = I2C_MASTER_NUM,
        .scl_io_num = I2C_MASTER_SCL_IO,
        .sda_io_num = I2C_MASTER_SDA_IO,
        .glitch_ignore_cnt = 7,
        .flags.enable_internal_pullup = 1,
    };

    i2c_master_bus_handle_t i2c_bus_handle;
    ESP_ERROR_CHECK(i2c_new_master_bus(&i2c_master_bus_config, &i2c_bus_handle));

    ESP_LOGI(TAG, "I2C master bus instantaited");

    return i2c_bus_handle;
}

i2c_master_dev_handle_t config_i2c_device(i2c_master_bus_handle_t bus_handle)
{
    i2c_device_config_t bme280_config = {
        .dev_addr_length = I2C_ADDR_BIT_7,
        .device_address = BME280_ADDRESS,
        .scl_speed_hz = I2C_MASTER_FREQ_HZ,
    };

    i2c_master_dev_handle_t dev_handle;
    ESP_ERROR_CHECK(i2c_master_bus_add_device(bus_handle, &bme280_config, &dev_handle));

    ESP_LOGI(TAG, "I2C bme280 device instantiated and added to bus instance");

    return dev_handle;
}

void app_main(void)
{
    i2c_master_bus_handle_t bus_handle = config_i2c_bus();
    i2c_master_dev_handle_t dev_handle = config_i2c_device(bus_handle);

    // Now following the example in utkumaden code for using the esp idf bmx280 driver:
    bmx280_t *bmx280 = bmx280_create(I2C_NUM_0);

    if (!bmx280)
    {
        ESP_LOGE("test", "Could not create bmx280 driver.");
        return;
    }

    ESP_ERROR_CHECK(bmx280_init(bmx280, dev_handle));

    bmx280_config_t bmx_cfg = BMX280_DEFAULT_CONFIG;
    ESP_ERROR_CHECK(bmx280_configure(bmx280, &bmx_cfg));

    while (1)
    {
        ESP_ERROR_CHECK(bmx280_setMode(bmx280, BMX280_MODE_FORCE));
        do
        {
            vTaskDelay(pdMS_TO_TICKS(1));
        } while (bmx280_isSampling(bmx280));

        float temp = 0, pres = 0, hum = 0;
        ESP_ERROR_CHECK(bmx280_readoutFloat(bmx280, &temp, &pres, &hum));

        ESP_LOGI("test", "Read Values: temp = %f, pres = %f, hum = %f", temp, pres, hum);
    }
}
