cmake_minimum_required(VERSION 3.17)
set(CMAKE_C_STANDARD 11)

if ("${CONFIG_USE_I2C_LEGACY_DRIVER}")
    if ("$ENV{IDF_VERSION}" VERSION_GREATER_EQUAL "7.0.0")
        message(SEND_ERROR "[ BMX280 ] The legacy I2C driver has been removed since version 7.0.0. Unless you intended to compile for older IDF versions, change your build configuration to use the I2C master driver.")
    else()
        message(WARNING "[ BMX280 ] The legacy I2C driver has been deprecated since version 6.0.0. The driver will not compile with the legacy I2C driver enabled in version 7.0.0 or later.")
    endif()

    message(STATUS "[ BMX280 ] Using the legacy I2C driver.")
    set(BMX280_I2C_DRIVER_REQUIRE "driver")
else()
    message(STATUS "[ BMX280 ] Using the new I2C master driver.")
    set(BMX280_I2C_DRIVER_REQUIRE "esp_driver_i2c")
endif()

idf_component_register(
    SRCS bmx280.c
    INCLUDE_DIRS "include"
    REQUIRES "${BMX280_I2C_DRIVER_REQUIRE}"
)
