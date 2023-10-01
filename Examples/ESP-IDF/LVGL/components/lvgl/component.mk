#
# Component Makefile
#

# Set simple includes as default
ifndef LV_CONF_INCLUDE_SIMPLE
CFLAGS += -DLV_CONF_INCLUDE_SIMPLE
endif

COMPONENT_SRCDIRS := lvgl/ \
	lvgl/src/lv_core \
	lvgl/src/lv_draw \
	lvgl/src/lv_widgets \
	lvgl/src/lv_hal \
	lvgl/src/lv_misc \
	lvgl/src/lv_themes \
	lvgl/src/lv_font
COMPONENT_ADD_INCLUDEDIRS := $(COMPONENT_SRCDIRS) .
