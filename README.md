BMX280 for ESP-IDF
==================

BMX280 is a basic I2C based driver for ESP32 devices licensed mostly under MIT.
(See caption "License" for details.)

This driver can be used with either the legacy i2c.h driver, or the new i2c_master.h driver which is supported by ESP-IDF versions 5.3.1 and greater.

Espressif notes that both drivers CANNOT coexist, and as such this code is filled with ```'#if'``` compiler directives that reveal and hide code associated with the use of the selected driver.

Usage
-----

Clone this repository or add it as a submodule into your components directory.
Add the module as a requirement to your main module, or other modules.

Example Code
------------

For use with Legacy i2c.h driver

```c
#include "esp_log.h"
#include "bmx280.h"

void app_main(void)
{
    // Entry Point
    //ESP_ERROR_CHECK(nvs_flash_init());
    i2c_config_t i2c_cfg = {
        .mode = I2C_MODE_MASTER,
        .sda_io_num = GPIO_NUM_17,
        .scl_io_num = GPIO_NUM_16,
        .sda_pullup_en = false,
        .scl_pullup_en = false,

        .master = {
            .clk_speed = 100000
        }
    };

    ESP_ERROR_CHECK(i2c_param_config(I2C_NUM_0, &i2c_cfg));
    ESP_ERROR_CHECK(i2c_driver_install(I2C_NUM_0, I2C_MODE_MASTER, 0, 0, 0));

    bmx280_t* bmx280 = bmx280_create(I2C_NUM_0);

    if (!bmx280) { 
        ESP_LOGE("test", "Could not create bmx280 driver.");
        return;
    }

    ESP_ERROR_CHECK(bmx280_init(bmx280));

    bmx280_config_t bmx_cfg = BMX280_DEFAULT_CONFIG;
    ESP_ERROR_CHECK(bmx280_configure(bmx280, &bmx_cfg));

    while (1)
    {
        ESP_ERROR_CHECK(bmx280_setMode(bmx280, BMX280_MODE_FORCE));
        do {
            vTaskDelay(pdMS_TO_TICKS(1));
        } while(bmx280_isSampling(bmx280));

        float temp = 0, pres = 0, hum = 0;
        ESP_ERROR_CHECK(bmx280_readoutFloat(bmx280, &temp, &pres, &hum));

        ESP_LOGI("test", "Read Values: temp = %f, pres = %f, hum = %f", temp, pres, hum);
    }
}
```

For use with i2c_master.h driver please see the [example](examples/example_i2c_master.c) here

Note
-------

While using the new I2C master driver with ESP-IDF version 5.3.1 gave me a watchdog timeout error causing the device to panic.
The only way to resolve this issue is to follow the advice of Mythbuster5 in the discussion on this issue post: <https://github.com/espressif/esp-idf/issues/12929>

You must comment out or delete the following line:

```c
ret = esp_intr_alloc_intrstatus(i2c_periph_signal[i2c_port_num].irq, isr_flags, (uint32_t)i2c_ll_get_interrupt_status_reg(hal->dev), I2C_LL_MASTER_EVENT_INTR, i2c_master_isr_handler_default, i2c_master, &i2c_master->base->intr_handle);
ESP_GOTO_ON_ERROR(ret, err, TAG, "install i2c master interrupt failed");
atomic_init(&i2c_master->status, I2C_STATUS_IDLE);

// **i2c_ll_enable_intr_mask(hal->dev, I2C_LL_MASTER_EVENT_INTR); This line!!!**
i2c_ll_master_set_filter(hal->dev, bus_config->glitch_ignore_cnt);

xSemaphoreGive(i2c_master->cmd_semphr);
```

TODO
-------

- Modify the USE_I2C_MASTER_DRIVER define statement to work with sdkconfig to allow user to easily select between them without modifying bmx280.h source code

License
-------

This repository contains a lot of code I have written which is licensed under
MIT, however there are sections modified from the BME280 datasheet which is
unclearly licensed.

The unclearly licensed section is clearly marked with two comments. Any code
between `// HERE BE DRAGONS` and `// END OF DRAGONS` contains modified versions
of the Bosch Sensortec code.

Please take note of this in production.
