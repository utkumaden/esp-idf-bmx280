#include "esp_log.h"
#include "bmx280.h"
#include "driver/i2c_types.h"

#define I2C_PORT_AUTO -1
#define BMX280_SDA_NUM GPIO_NUM_13
#define BMX280_SCL_NUM GPIO_NUM_14

i2c_master_bus_handle_t i2c_bus_init(uint8_t sda_io, uint8_t scl_io)
{
    i2c_master_bus_config_t i2c_bus_config = {
        .i2c_port = I2C_PORT_AUTO,
        .sda_io_num = sda_io,
        .scl_io_num = scl_io,
        .clk_source = I2C_CLK_SRC_DEFAULT,
        .glitch_ignore_cnt = 7,
        .flags.enable_internal_pullup = true,
    };
    i2c_master_bus_handle_t bus_handle;
    ESP_ERROR_CHECK(i2c_new_master_bus(&i2c_bus_config, &bus_handle));
    ESP_LOGI("test","I2C master bus created");
    return bus_handle;
}

esp_err_t bmx280_dev_init(bmx280_t** bmx280,i2c_master_bus_handle_t bus_handle)
{
    *bmx280 = bmx280_create_master(bus_handle);
    if (!*bmx280) { 
        ESP_LOGE("test", "Could not create bmx280 driver.");
        return ESP_FAIL;
    }
    
    ESP_ERROR_CHECK(bmx280_init(*bmx280));
    bmx280_config_t bmx_cfg = BMX280_DEFAULT_CONFIG;
    ESP_ERROR_CHECK(bmx280_configure(*bmx280, &bmx_cfg));
    return ESP_OK;
}

void app_main(void)
{
    // Entry Point
    //ESP_ERROR_CHECK(nvs_flash_init());
    i2c_master_bus_handle_t bus_handle = i2c_bus_init(BMX280_SDA_NUM, BMX280_SCL_NUM);
    bmx280_t* bmx280 = NULL;
    ESP_ERROR_CHECK(bmx280_dev_init(&bmx280,bus_handle));

    ESP_ERROR_CHECK(bmx280_setMode(bmx280, BMX280_MODE_CYCLE));
    float temp = 0, pres = 0, hum = 0;
    for(int i = 0; i < 10; i++)
    {
        do {
            vTaskDelay(pdMS_TO_TICKS(1));
        } while(bmx280_isSampling(bmx280));

        ESP_ERROR_CHECK(bmx280_readoutFloat(bmx280, &temp, &pres, &hum));
        ESP_LOGI("test", "Read Values: temp = %f, pres = %f, hum = %f", temp, pres, hum);
        vTaskDelay(pdMS_TO_TICKS(1000));
    }
    
    bmx280_close(bmx280);
    i2c_del_master_bus(bus_handle);
    ESP_LOGI("test", "Restarting now.");
    esp_restart();
}