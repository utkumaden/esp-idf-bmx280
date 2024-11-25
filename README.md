BMX280 for ESP-IDF
==================
BMX280 is a basic I2C based driver for ESP32 devices licensed mostly under MIT.
(See caption "License" for details.)

Usage
-----
Clone this repository or add it as a submodule into your components directory.
Add the module as a requirement to your main module, or other modules.

Example Code
------------
To use the legacy i2c.h driver, please refer to the [example_with_i2c.h](examples/bmx280_example_without_i2c_master.c).

To use the i2c_master.h driver (ESP-IDF >= 5.3):

* First, run `idf.py menuconfig` to configure the project.

* Navigate to Component Config -> BMX280 Options -> I2C driver setting -> I2C Master Driver (i2c_master.h).

* Press the 'S' key to save the configuration.

* You can then use the I2C Master driver to complete your code.

For example code, please see the [example_with_i2c_master.h](examples/bmx280_example_with_i2c_master.c).

**Note:** If you want to use the legacy i2c.h driver, no changes are needed; continue using `bmx280_create` to create the anonymous structure `bmx280`. If you use the i2c_master.h driver, you need to use `bmx280_create_master` to create the structure.

License
-------
This repository contains a lot of code I have written which is licensed under
MIT, however there are sections modified from the BME280 datasheet which is
unclearly licensed.

The unclearly licensed section is clearly marked with two comments. Any code
between `// HERE BE DRAGONS` and `// END OF DRAGONS` contains modified versions
of the Bosch Sensortec code.

Please take note of this in production.