
idf_component_register(
    SRCS bmx280.c
    INCLUDE_DIRS "include"
    REQUIRES driver
    REQUIRES esp_driver_i2c
)
