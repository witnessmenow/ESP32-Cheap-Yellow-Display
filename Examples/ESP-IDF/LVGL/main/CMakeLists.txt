set(SOURCES main.c )
idf_component_register(SRCS ${SOURCES}
                    INCLUDE_DIRS .
                    REQUIRES lvgl_esp32_drivers 
                             lvgl_touch lvgl_tft
                             lvgl
                             lv_examples
                             driver
                             esp_event
                             esp_timer
                             )

target_compile_definitions(${COMPONENT_LIB} PRIVATE LV_CONF_INCLUDE_SIMPLE=1)
