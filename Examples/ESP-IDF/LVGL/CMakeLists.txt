cmake_minimum_required(VERSION 3.5)

include($ENV{IDF_PATH}/tools/cmake/project.cmake)
list(APPEND EXTRA_COMPONENT_DIRS components/lvgl_esp32_drivers) 
list(APPEND EXTRA_COMPONENT_DIRS components/lvgl_esp32_drivers/lvgl_touch)
list(APPEND EXTRA_COMPONENT_DIRS components/lvgl_esp32_drivers/lvgl_tft)

if (NOT DEFINED PROJECT_NAME)
	project(FULL_Test)
endif (NOT DEFINED PROJECT_NAME)
