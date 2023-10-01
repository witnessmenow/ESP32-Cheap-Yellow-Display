#
# Component Makefile
#

CFLAGS += -DLV_CONF_INCLUDE_SIMPLE

COMPONENT_SRCDIRS := lv_examples           \
    lv_examples/src/lv_demo_benchmark      \
    lv_examples/src/lv_demo_keypad_encoder \
    lv_examples/src/demo_stress            \
    lv_examples/src/lv_demo_widgets        \
    lv_examples/src/lv_ex_style            \
    lv_examples/src/lv_ex_widgets          \
    lv_examples/assets

COMPONENT_ADD_INCLUDEDIRS := $(COMPONENT_SRCDIRS) .
