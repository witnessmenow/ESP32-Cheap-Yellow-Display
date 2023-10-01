set(SOURCES "touch_driver.c")

# Include only the source file of the selected
# display controller.
if(CONFIG_LVGL_TOUCH_CONTROLLER_XPT2046)
    list(APPEND SOURCES "xpt2046.c")
elseif(CONFIG_LVGL_TOUCH_CONTROLLER_FT6X06)
    list(APPEND SOURCES "ft6x36.c")
elseif(CONFIG_LVGL_TOUCH_CONTROLLER_STMPE610)
    list(APPEND SOURCES "stmpe610")
elseif(CONFIG_LVGL_TOUCH_CONTROLLER_ADCRAW)
    list(APPEND SOURCES "adcraw.c")
elseif(CONFIG_LVGL_TOUCH_CONTROLLER_FT81X)
    list(APPEND SOURCES "FT81x.c")
elseif(CONFIG_LVGL_TOUCH_CONTROLLER_RA8875)
    list(APPEND SOURCES "ra8875_touch.c")
endif()

if(CONFIG_LVGL_TOUCH_DRIVER_PROTOCOL_SPI)
    list(APPEND SOURCES "tp_spi.c")
elseif(CONFIG_LVGL_TOUCH_DRIVER_PROTOCOL_I2C)
    list(APPEND SOURCES "tp_i2c.c")
endif()

# Print the included source files
message("SOURCES contents: " "${SOURCES}")

idf_component_register(SRCS ${SOURCES}
                       INCLUDE_DIRS .
                       REQUIRES lvgl)
