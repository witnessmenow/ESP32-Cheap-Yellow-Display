/**
 * @file lv_conf.h
 *
 */

/*
 * COPY THIS FILE AS `lv_conf.h` NEXT TO the `lvgl` FOLDER
 */

#if 1 /*Set it to "1" to enable content*/

#ifndef LV_CONF_H
#define LV_CONF_H
/* clang-format off */

#include <stdint.h>

#include "esp_attr.h"

/*====================
   Graphical settings
 *====================*/

/* Maximal horizontal and vertical resolution to support by the library.*/
#if defined (CONFIG_LVGL_PREDEFINED_DISPLAY_NONE) && defined (CONFIG_LVGL_DISPLAY_ORIENTATION_PORTRAIT)
    #define LV_VER_RES_MAX          (CONFIG_LVGL_DISPLAY_WIDTH)
    #define LV_HOR_RES_MAX          (CONFIG_LVGL_DISPLAY_HEIGHT)
#elif defined (CONFIG_LVGL_PREDEFINED_DISPLAY_NONE) && defined (CONFIG_LVGL_DISPLAY_ORIENTATION_PORTRAIT_INVERTED)
    #define LV_VER_RES_MAX          (CONFIG_LVGL_DISPLAY_WIDTH)
    #define LV_HOR_RES_MAX          (CONFIG_LVGL_DISPLAY_HEIGHT)
#elif defined (CONFIG_LVGL_PREDEFINED_DISPLAY_NONE) && defined (CONFIG_LVGL_DISPLAY_ORIENTATION_LANDSCAPE)
    #define LV_HOR_RES_MAX          (CONFIG_LVGL_DISPLAY_WIDTH)
    #define LV_VER_RES_MAX          (CONFIG_LVGL_DISPLAY_HEIGHT)
#elif defined (CONFIG_LVGL_PREDEFINED_DISPLAY_NONE) && defined (CONFIG_LVGL_DISPLAY_ORIENTATION_LANDSCAPE_INVERTED)
    #define LV_HOR_RES_MAX          (CONFIG_LVGL_DISPLAY_WIDTH)
    #define LV_VER_RES_MAX          (CONFIG_LVGL_DISPLAY_HEIGHT)
#else
    #define LV_HOR_RES_MAX          (CONFIG_LVGL_DISPLAY_WIDTH)
    #define LV_VER_RES_MAX          (CONFIG_LVGL_DISPLAY_HEIGHT)
#endif

/* Color depth:
 * - 1:  1 byte per pixel
 * - 8:  RGB233
 * - 16: RGB565
 * - 32: ARGB8888
 */
#if defined CONFIG_LVGL_TFT_DISPLAY_MONOCHROME
/* For the monochrome display driver controller, e.g. SSD1306 and SH1107, use a color depth of 1. */
#define LV_COLOR_DEPTH     1
#else
#define LV_COLOR_DEPTH     16
#endif

/* Swap the 2 bytes of RGB565 color.
 * Useful if the display has a 8 bit interface (e.g. SPI)*/
#if defined CONFIG_LVGL_TFT_DISPLAY_CONTROLLER_ILI9341
#define LV_COLOR_16_SWAP   1
#elif defined CONFIG_LVGL_TFT_DISPLAY_CONTROLLER_ILI9488
#define LV_COLOR_16_SWAP   0
#elif defined CONFIG_LVGL_TFT_DISPLAY_CONTROLLER_ST7789
#define LV_COLOR_16_SWAP   1
#elif defined CONFIG_LVGL_TFT_DISPLAY_CONTROLLER_ST7735S
#define LV_COLOR_16_SWAP   1
#elif defined CONFIG_LVGL_TFT_DISPLAY_CONTROLLER_HX8357
#define LV_COLOR_16_SWAP   1
#elif defined CONFIG_LVGL_TFT_DISPLAY_CONTROLLER_SH1107
#define LV_COLOR_16_SWAP   0
#elif defined CONFIG_LVGL_TFT_DISPLAY_CONTROLLER_SSD1306
#define LV_COLOR_16_SWAP   0
#elif defined CONFIG_LVGL_TFT_DISPLAY_CONTROLLER_ILI9486
#define LV_COLOR_16_SWAP   1
#elif defined CONFIG_LVGL_TFT_DISPLAY_CONTROLLER_FT81X
#define LV_COLOR_16_SWAP   0
#elif defined CONFIG_LVGL_TFT_DISPLAY_CONTROLLER_RA8875
#define LV_COLOR_16_SWAP   1
#endif

/* 1: Enable screen transparency.
 * Useful for OSD or other overlapping GUIs.
 * Requires `LV_COLOR_DEPTH = 32` colors and the screen's style should be modified: `style.body.opa = ...`*/
#define LV_COLOR_SCREEN_TRANSP    0

/*Images pixels with this color will not be drawn (with chroma keying)*/
#define LV_COLOR_TRANSP    LV_COLOR_LIME         /*LV_COLOR_LIME: pure green*/

/* Enable anti-aliasing (lines, and radiuses will be smoothed) */
#define LV_ANTIALIAS        1

/* Default display refresh period.
 * Can be changed in the display driver (`lv_disp_drv_t`).*/
#define LV_DISP_DEF_REFR_PERIOD      30      /*[ms]*/

/* Dot Per Inch: used to initialize default sizes.
 * E.g. a button with width = LV_DPI / 2 -> half inch wide
 * (Not so important, you can adjust it to modify default sizes and spaces)*/
#define LV_DPI              100     /*[px]*/

/* The the real width of the display changes some default values:
 * default object sizes, layout of examples, etc.
 * According to the width of the display (hor. res. / dpi)
 * the displays fall in 4 categories.
 * The 4th is extra large which has no upper limit so not listed here
 * The upper limit of the categories are set below in 0.1 inch unit.
 */
#define LV_DISP_SMALL_LIMIT  30
#define LV_DISP_MEDIUM_LIMIT 50
#define LV_DISP_LARGE_LIMIT  70

/* Type of coordinates. Should be `int16_t` (or `int32_t` for extreme cases) */
typedef int16_t lv_coord_t;

/*=========================
   Memory manager settings
 *=========================*/

/* LittelvGL's internal memory manager's settings.
 * The graphical objects and other related data are stored here. */

/* 1: use custom malloc/free, 0: use the built-in `lv_mem_alloc` and `lv_mem_free` */
#define LV_MEM_CUSTOM      0
#if LV_MEM_CUSTOM == 0
/* Size of the memory used by `lv_mem_alloc` in bytes (>= 2kB)*/
#  define LV_MEM_SIZE    ( CONFIG_LVGL_MEM_SIZE * 1024U)

/* Complier prefix for a big array declaration */
#  define LV_MEM_ATTR

/* Set an address for the memory pool instead of allocating it as an array.
 * Can be in external SRAM too. */
#  define LV_MEM_ADR          0

/* Automatically defrag. on free. Defrag. means joining the adjacent free cells. */
#  define LV_MEM_AUTO_DEFRAG  1
#else       /*LV_MEM_CUSTOM*/
#  define LV_MEM_CUSTOM_INCLUDE <stdlib.h>   /*Header for the dynamic memory function*/
#  define LV_MEM_CUSTOM_ALLOC   malloc       /*Wrapper to malloc*/
#  define LV_MEM_CUSTOM_FREE    free         /*Wrapper to free*/
#endif     /*LV_MEM_CUSTOM*/

/* Garbage Collector settings
 * Used if lvgl is binded to higher level language and the memory is managed by that language */
#define LV_ENABLE_GC 0
#if LV_ENABLE_GC != 0
#  define LV_GC_INCLUDE "gc.h"                           /*Include Garbage Collector related things*/
#  define LV_MEM_CUSTOM_REALLOC   your_realloc           /*Wrapper to realloc*/
#  define LV_MEM_CUSTOM_GET_SIZE  your_mem_get_size      /*Wrapper to lv_mem_get_size*/
#endif /* LV_ENABLE_GC */

/*=======================
   Input device settings
 *=======================*/

/* Input device default settings.
 * Can be changed in the Input device driver (`lv_indev_drv_t`)*/

/* Input device read period in milliseconds */
#define LV_INDEV_DEF_READ_PERIOD          CONFIG_LVGL_INDEV_DEF_READ_PERIOD

/* Drag threshold in pixels */
#define LV_INDEV_DEF_DRAG_LIMIT           CONFIG_LVGL_INDEV_DEF_DRAG_LIMIT

/* Drag throw slow-down in [%]. Greater value -> faster slow-down */
#define LV_INDEV_DEF_DRAG_THROW           CONFIG_LVGL_INDEV_DEF_DRAG_THROW

/* Long press time in milliseconds.
 * Time to send `LV_EVENT_LONG_PRESSSED`) */
#define LV_INDEV_DEF_LONG_PRESS_TIME      CONFIG_LVGL_INDEV_DEF_LONG_PRESS_TIME

/* Repeated trigger period in long press [ms]
 * Time between `LV_EVENT_LONG_PRESSED_REPEAT */
#define LV_INDEV_DEF_LONG_PRESS_REP_TIME  CONFIG_LVGL_INDEV_DEF_LONG_PRESS_REP_TIME

/* Gesture threshold in pixels */
#define LV_INDEV_DEF_GESTURE_LIMIT        CONFIG_LVGL_INDEV_DEF_GESTURE_LIMIT

/* Gesture min velocity at release before swipe (pixels)*/
#define LV_INDEV_DEF_GESTURE_MIN_VELOCITY CONFIG_LVGL_INDEV_DEF_GESTURE_MIN_VELOCITY

/*==================
 * Feature usage
 *==================*/

/*1: Enable the Animations */
#if defined CONFIG_LVGL_FEATURE_USE_ANIMATION
    #define LV_USE_ANIMATION        1
#else
    #define LV_USE_ANIMATION        0
#endif

#if LV_USE_ANIMATION

/*Declare the type of the user data of animations (can be e.g. `void *`, `int`, `struct`)*/
typedef void * lv_anim_user_data_t;

#endif

/* 1: Enable shadow drawing*/
#if defined CONFIG_LVGL_FEATURE_USE_SHADOW
    #define LV_USE_SHADOW           1
#else
    #define LV_USE_SHADOW           0
#endif

#if LV_USE_SHADOW
/* Allow buffering some shadow calculation
 * LV_SHADOW_CACHE_SIZE is the max. shadow size to buffer,
 * where shadow size is `shadow_width + radius`
 * Caching has LV_SHADOW_CACHE_SIZE^2 RAM cost*/
#define LV_SHADOW_CACHE_SIZE    0
#endif

/* 1: Use other blend modes than normal (`LV_BLEND_MODE_...`)*/
#if defined CONFIG_LVGL_FEATURE_USE_BLEND_MODES
    #define LV_USE_BLEND_MODES      1
#else
    #define LV_USE_BLEND_MODES      0
#endif

/* 1: Use the `opa_scale` style property to set the opacity of an object and its children at once*/
#if defined CONFIG_LVGL_FEATURE_USE_OPA_SCALE
    #define LV_USE_OPA_SCALE        1
#else
    #define LV_USE_OPA_SCALE        0
#endif

/* 1: Use image zoom and rotation*/
#if defined CONFIG_LVGL_FEATURE_USE_IMG_TRANSFORM
    #define LV_USE_IMG_TRANSFORM    1
#else
    #define LV_USE_IMG_TRANSFORM    0
#endif

/* 1: Enable object groups (for keyboard/encoder navigation) */
#if defined CONFIG_LVGL_FEATURE_USE_GROUP
    #define LV_USE_GROUP            1
#else
    #define LV_USE_GROUP            0
#endif

#if LV_USE_GROUP
typedef void * lv_group_user_data_t;
#endif  /*LV_USE_GROUP*/

/* 1: Enable GPU interface
 * Only enables `gpu_fill_cb` and `gpu_blend_cb` in the disp. drv- */
#if defined CONFIG_LVGL_FEATURE_USE_GPU
    #define LV_USE_GPU              1
#else
    #define LV_USE_GPU              0
#endif

#if defined CONFIG_LVGL_FEATURE_USE_GPU_STM32_DMA2D
    #define LV_USE_GPU_STM32_DMA2D  1
#else
    #define LV_USE_GPU_STM32_DMA2D  0
#endif

/* 1: Enable file system (might be required for images */
#if defined CONFIG_LVGL_FEATURE_USE_FILESYSTEM
    #define LV_USE_FILESYSTEM       1
#else
    #define LV_USE_FILESYSTEM       0
#endif

#if LV_USE_FILESYSTEM
/*Declare the type of the user data of file system drivers (can be e.g. `void *`, `int`, `struct`)*/
typedef void * lv_fs_drv_user_data_t;
#endif

/*1: Add a `user_data` to drivers and objects*/
#if defined CONFIG_LVGL_FEATURE_USE_USER_DATA
    #define LV_USE_USER_DATA        1
#else
    #define LV_USE_USER_DATA        0
#endif

/*1: Show CPU usage and FPS count in the right bottom corner*/
#if defined CONFIG_LVGL_FEATURE_USE_PERF_MONITOR
    #define LV_USE_PERF_MONITOR     1
#else
    #define LV_USE_PERF_MONITOR     0
#endif

/*1: Use the functions and types from the older API if possible */
#if defined CONFIG_LVGL_FEATURE_USE_API_EXTENSION_V6
    #define LV_USE_API_EXTENSION_V6  1
#else
    #define LV_USE_API_EXTENSION_V6  0
#endif

/*========================
 * Image decoder and cache
 *========================*/

/* 1: Enable indexed (palette) images */
#if defined CONFIG_LVGL_IMG_CF_INDEXED
    #define LV_IMG_CF_INDEXED   1
#else
    #define LV_IMG_CF_INDEXED   0
#endif

/* 1: Enable alpha indexed images */
#if defined CONFIG_LVGL_IMG_CF_ALPHA
    #define LV_IMG_CF_ALPHA     1
#else
    #define LV_IMG_CF_ALPHA     0
#endif

/* Default image cache size. Image caching keeps the images opened.
 * If only the built-in image formats are used there is no real advantage of caching.
 * (I.e. no new image decoder is added)
 * With complex image decoders (e.g. PNG or JPG) caching can save the continuous open/decode of images.
 * However the opened images might consume additional RAM.
 * LV_IMG_CACHE_DEF_SIZE must be >= 1 */
#define LV_IMG_CACHE_DEF_SIZE   CONFIG_LVGL_IMG_CACHE_DEF_SIZE

/*Declare the type of the user data of image decoder (can be e.g. `void *`, `int`, `struct`)*/
typedef void * lv_img_decoder_user_data_t;

/*=====================
 *  Compiler settings
 *====================*/
/* Define a custom attribute to `lv_tick_inc` function */
#define LV_ATTRIBUTE_TICK_INC

/* Define a custom attribute to `lv_task_handler` function */
#define LV_ATTRIBUTE_TASK_HANDLER

/* Define a custom attribute to `lv_disp_flush_ready` function */
#define LV_ATTRIBUTE_FLUSH_READY

/* With size optimization (-Os) the compiler might not align data to
 * 4 or 8 byte boundary. This alignment will be explicitly applied where needed.
 * E.g. __attribute__((aligned(4))) */
#define LV_ATTRIBUTE_MEM_ALIGN

/* Attribute to mark large constant arrays for example
 * font's bitmaps */
#define LV_ATTRIBUTE_LARGE_CONST

/* Prefix performance critical functions to place them into a faster memory (e.g RAM)
 * Uses 15-20 kB extra memory */
#define LV_ATTRIBUTE_FAST_MEM

/* Export integer constant to binding.
 * This macro is used with constants in the form of LV_<CONST> that
 * should also appear on lvgl binding API such as Micropython
 *
 * The default value just prevents a GCC warning.
 */
#define LV_EXPORT_CONST_INT(int_value) struct _silence_gcc_warning

/*===================
 *  HAL settings
 *==================*/

/* 1: use a custom tick source.
 * It removes the need to manually update the tick with `lv_tick_inc`) */
#define LV_TICK_CUSTOM     0
#if LV_TICK_CUSTOM == 1
#define LV_TICK_CUSTOM_INCLUDE  "something.h"       /*Header for the sys time function*/
#define LV_TICK_CUSTOM_SYS_TIME_EXPR (millis())     /*Expression evaluating to current systime in ms*/
#endif   /*LV_TICK_CUSTOM*/

typedef void * lv_disp_drv_user_data_t;             /*Type of user data in the display driver*/
typedef void * lv_indev_drv_user_data_t;            /*Type of user data in the input device driver*/

/*================
 * Log settings
 *===============*/

/*1: Enable the log module*/
#if defined CONFIG_LVGL_USE_LOG
#  define LV_USE_LOG 1
#else
#  define LV_USE_LOG 0
#endif

#if LV_USE_LOG
/* How important log should be added:
 * LV_LOG_LEVEL_TRACE       A lot of logs to give detailed information
 * LV_LOG_LEVEL_INFO        Log important events
 * LV_LOG_LEVEL_WARN        Log if something unwanted happened but didn't cause a problem
 * LV_LOG_LEVEL_ERROR       Only critical issue, when the system may fail
 * LV_LOG_LEVEL_NONE        Do not log anything
 */
#if defined CONFIG_LVGL_LOG_LEVEL_TRACE
#  define LV_LOG_LEVEL    LV_LOG_LEVEL_TRACE
#elif defined CONFIG_LVGL_LOG_LEVEL_INFO
#  define LV_LOG_LEVEL    LV_LOG_LEVEL_INFO
#elif defined CONFIG_LVGL_LOG_LEVEL_WARN
#  define LV_LOG_LEVEL    LV_LOG_LEVEL_WARN
#elif defined CONFIG_LVGL_LOG_LEVEL_ERROR
#  define LV_LOG_LEVEL    LV_LOG_LEVEL_ERROR
#elif defined CONFIG_LVGL_LOG_LEVEL_NONE
#  define LV_LOG_LEVEL    LV_LOG_LEVEL_NONE
#endif

/* 1: Print the log with 'printf';
 * 0: user need to register a callback with `lv_log_register_print_cb`*/
#if defined CONFIG_LVGL_LOG_PRINTF
#  define LV_LOG_PRINTF   1
#else
#  define LV_LOG_PRINTF   0
#endif
#endif  /*LV_USE_LOG*/

/*=================
 * Debug settings
 *================*/

/* If Debug is enabled LittelvGL validates the parameters of the functions.
 * If an invalid parameter is found an error log message is printed and
 * the MCU halts at the error. (`LV_USE_LOG` should be enabled)
 * If you are debugging the MCU you can pause
 * the debugger to see exactly where  the issue is.
 *
 * The behavior of asserts can be overwritten by redefining them here.
 * E.g. #define LV_ASSERT_MEM(p)  <my_assert_code>
 */

#if defined CONFIG_LVGL_USE_DEBUG
#  define LV_USE_DEBUG 1
#else
#  define LV_USE_DEBUG 0
#endif

#if LV_USE_DEBUG

/*Check if the parameter is NULL. (Quite fast) */
#if defined CONFIG_LVGL_USE_ASSERT_NULL
#  define LV_USE_ASSERT_NULL      1
#else
#  define LV_USE_ASSERT_NULL      0
#endif

/*Checks is the memory is successfully allocated or no. (Quite fast)*/
#if defined CONFIG_LVGL_USE_ASSERT_MEM
#  define LV_USE_ASSERT_MEM       1
#else
#  define LV_USE_ASSERT_MEM       0
#endif

/*Check the integrity of `lv_mem` after critical operations. (Slow)*/
#if defined CONFIG_LVGL_USE_ASSERT_MEM_INTEGRITY
#  define LV_USE_ASSERT_MEM_INTEGRITY       1
#else
#  define LV_USE_ASSERT_MEM_INTEGRITY       0
#endif

/* Check the strings.
 * Search for NULL, very long strings, invalid characters, and unnatural repetitions. (Slow)
 * If disabled `LV_USE_ASSERT_NULL` will be performed instead (if it's enabled) */
#if defined CONFIG_LVGL_USE_ASSERT_STR
#  define LV_USE_ASSERT_STR       1
#else
#  define LV_USE_ASSERT_STR       0
#endif

/* Check NULL, the object's type and existence (e.g. not deleted). (Quite slow)
 * If disabled `LV_USE_ASSERT_NULL` will be performed instead (if it's enabled) */
#if defined CONFIG_LVGL_USE_ASSERT_OBJ
#  define LV_USE_ASSERT_OBJ       1
#else
#  define LV_USE_ASSERT_OBJ       0
#endif

/*Check if the styles are properly initialized. (Fast)*/
#if defined CONFIG_LVGL_USE_ASSERT_STYLE
#  define LV_USE_ASSERT_STYLE       1
#else
#  define LV_USE_ASSERT_STYLE       0
#endif

#endif /*LV_USE_DEBUG*/

/*==================
 *    FONT USAGE
 *===================*/

/* The built-in fonts contains the ASCII range and some Symbols with  4 bit-per-pixel.
 * The symbols are available via `LV_SYMBOL_...` defines
 * More info about fonts: https://docs.lvgl.com/#Fonts
 * To create a new font go to: https://lvgl.com/ttf-font-to-c-array
 */

/* Montserrat fonts with bpp = 4
 * https://fonts.google.com/specimen/Montserrat  */
#define LV_FONT_MONTSERRAT_12    CONFIG_LVGL_FONT_MONTSERRAT_12
#define LV_FONT_MONTSERRAT_14    CONFIG_LVGL_FONT_MONTSERRAT_14
#define LV_FONT_MONTSERRAT_16    CONFIG_LVGL_FONT_MONTSERRAT_16
#define LV_FONT_MONTSERRAT_18    CONFIG_LVGL_FONT_MONTSERRAT_18
#define LV_FONT_MONTSERRAT_20    CONFIG_LVGL_FONT_MONTSERRAT_20
#define LV_FONT_MONTSERRAT_22    CONFIG_LVGL_FONT_MONTSERRAT_22
#define LV_FONT_MONTSERRAT_24    CONFIG_LVGL_FONT_MONTSERRAT_24
#define LV_FONT_MONTSERRAT_26    CONFIG_LVGL_FONT_MONTSERRAT_26
#define LV_FONT_MONTSERRAT_28    CONFIG_LVGL_FONT_MONTSERRAT_28
#define LV_FONT_MONTSERRAT_30    CONFIG_LVGL_FONT_MONTSERRAT_30
#define LV_FONT_MONTSERRAT_32    CONFIG_LVGL_FONT_MONTSERRAT_32
#define LV_FONT_MONTSERRAT_34    CONFIG_LVGL_FONT_MONTSERRAT_34
#define LV_FONT_MONTSERRAT_36    CONFIG_LVGL_FONT_MONTSERRAT_36
#define LV_FONT_MONTSERRAT_38    CONFIG_LVGL_FONT_MONTSERRAT_38
#define LV_FONT_MONTSERRAT_40    CONFIG_LVGL_FONT_MONTSERRAT_40
#define LV_FONT_MONTSERRAT_42    CONFIG_LVGL_FONT_MONTSERRAT_42
#define LV_FONT_MONTSERRAT_44    CONFIG_LVGL_FONT_MONTSERRAT_44
#define LV_FONT_MONTSERRAT_46    CONFIG_LVGL_FONT_MONTSERRAT_46
#define LV_FONT_MONTSERRAT_48    CONFIG_LVGL_FONT_MONTSERRAT_48

/* Demonstrate special features */
#define LV_FONT_MONTSERRAT_12_SUBPX      CONFIG_LVGL_FONT_MONTSERRAT12SUBPX
#define LV_FONT_MONTSERRAT_28_COMPRESSED CONFIG_LVGL_FONT_MONTSERRAT28COMPRESSED  /*bpp = 3*/
#define LV_FONT_DEJAVU_16_PERSIAN_HEBREW CONFIG_LVGL_FONT_DEJAVU_16_PERSIAN_HEBREW  /*Hebrew, Arabic, PErisan letters and all their forms*/
#define LV_FONT_SIMSUN_16_CJK            CONFIG_LVGL_FONT_SIMSUN_16_CJK  /*1000 most common CJK radicals*/

/*Pixel perfect monospace font
 * http://pelulamu.net/unscii/ */
#define LV_FONT_UNSCII_8     CONFIG_LVGL_FONT_UNSCII8

/* Optionally declare your custom fonts here.
 * You can use these fonts as default font too
 * and they will be available globally. E.g.
 * #define LV_FONT_CUSTOM_DECLARE LV_FONT_DECLARE(my_font_1) \
 *                                LV_FONT_DECLARE(my_font_2)
 */
#define LV_FONT_CUSTOM_DECLARE

/* Enable it if you have fonts with a lot of characters.
 * The limit depends on the font size, font face and bpp
 * but with > 10,000 characters if you see issues probably you need to enable it.*/
#define LV_FONT_FMT_TXT_LARGE   0

/* Set the pixel order of the display.
 * Important only if "subpx fonts" are used.
 * With "normal" font it doesn't matter.
 */
#define LV_FONT_SUBPX_BGR    0

/*Declare the type of the user data of fonts (can be e.g. `void *`, `int`, `struct`)*/
typedef void * lv_font_user_data_t;

/*================
 *  THEME USAGE
 *================*/

/*Always enable at least on theme*/

/* No theme, you can apply your styles as you need
 * No flags. Set LV_THEME_DEFAULT_FLAG 0 */
 #define LV_USE_THEME_EMPTY       CONFIG_LVGL_THEME_EMPTY

/*Simple to the create your theme based on it
 * No flags. Set LV_THEME_DEFAULT_FLAG 0 */
 #define LV_USE_THEME_TEMPLATE    CONFIG_LVGL_THEME_TEMPLATE

/* A fast and impressive theme.
 * Flags:
 * LV_THEME_MATERIAL_FLAG_LIGHT: light theme
 * LV_THEME_MATERIAL_FLAG_DARK: dark theme*/
 #define LV_USE_THEME_MATERIAL    CONFIG_LVGL_THEME_MATERIAL

/* Mono-color theme for monochrome displays.
 * If LV_THEME_DEFAULT_COLOR_PRIMARY is LV_COLOR_BLACK the
 * texts and borders will be black and the background will be
 * white. Else the colors are inverted.
 * No flags. Set LV_THEME_DEFAULT_FLAG 0 */
 #define LV_USE_THEME_MONO        CONFIG_LVGL_THEME_MONO

#define LV_THEME_DEFAULT_INCLUDE            <stdint.h>      /*Include a header for the init. function*/

#if defined (CONFIG_LVGL_THEME_DEFAULT_INIT_EMPTY)
    #define LV_THEME_DEFAULT_INIT               lv_theme_empty_init
#elif defined (CONFIG_LVGL_THEME_DEFAULT_INIT_TEMPLATE)
    #define LV_THEME_DEFAULT_INIT               lv_theme_template_init
#elif defined (CONFIG_LVGL_THEME_DEFAULT_INIT_MATERIAL)
    #define LV_THEME_DEFAULT_INIT               lv_theme_material_init
#elif defined (CONFIG_LVGL_THEME_DEFAULT_INIT_MONO)
    #define LV_THEME_DEFAULT_INIT               lv_theme_mono_init
#else
    #error "Select default init"
#endif

#if !defined (CONFIG_LVGL_THEME_MONO)
    /* LV_THEME_DEFAULT_COLOR_PRIMARY */
    #if defined CONFIG_LVGL_THEME_DEFAULT_PRIMARY_COLOR_WHITE
        #define LV_THEME_DEFAULT_COLOR_PRIMARY      LV_COLOR_WHITE
    #elif defined CONFIG_LVGL_THEME_DEFAULT_PRIMARY_COLOR_SILVER
        #define LV_THEME_DEFAULT_COLOR_PRIMARY      LV_COLOR_SILVER
    #elif defined CONFIG_LVGL_THEME_DEFAULT_PRIMARY_COLOR_GRAY
        #define LV_THEME_DEFAULT_COLOR_PRIMARY      LV_COLOR_GRAY
    #elif defined CONFIG_LVGL_THEME_DEFAULT_PRIMARY_COLOR_BLACK
        #define LV_THEME_DEFAULT_COLOR_PRIMARY      LV_COLOR_BLACK
    #elif defined CONFIG_LVGL_THEME_DEFAULT_PRIMARY_COLOR_RED
        #define LV_THEME_DEFAULT_COLOR_PRIMARY      LV_COLOR_RED
    #elif defined CONFIG_LVGL_THEME_DEFAULT_PRIMARY_COLOR_MAROON
        #define LV_THEME_DEFAULT_COLOR_PRIMARY      LV_COLOR_MAROON
    #elif defined CONFIG_LVGL_THEME_DEFAULT_PRIMARY_COLOR_YELLOW
        #define LV_THEME_DEFAULT_COLOR_PRIMARY      LV_COLOR_YELLOW
    #elif defined CONFIG_LVGL_THEME_DEFAULT_PRIMARY_COLOR_OLIVE
        #define LV_THEME_DEFAULT_COLOR_PRIMARY      LV_COLOR_OLIVE
    #elif defined CONFIG_LVGL_THEME_DEFAULT_PRIMARY_COLOR_LIME
        #define LV_THEME_DEFAULT_COLOR_PRIMARY      LV_COLOR_LIME
    #elif defined CONFIG_LVGL_THEME_DEFAULT_PRIMARY_COLOR_GREEN
        #define LV_THEME_DEFAULT_COLOR_PRIMARY      LV_COLOR_GREEN
    #elif defined CONFIG_LVGL_THEME_DEFAULT_PRIMARY_COLOR_CYAN
        #define LV_THEME_DEFAULT_COLOR_PRIMARY      LV_COLOR_CYAN
    #elif defined CONFIG_LVGL_THEME_DEFAULT_PRIMARY_COLOR_AQUA
        #define LV_THEME_DEFAULT_COLOR_PRIMARY      LV_COLOR_AQUA
    #elif defined CONFIG_LVGL_THEME_DEFAULT_PRIMARY_COLOR_TEAL
        #define LV_THEME_DEFAULT_COLOR_PRIMARY      LV_COLOR_TEAL
    #elif defined CONFIG_LVGL_THEME_DEFAULT_PRIMARY_COLOR_BLUE
        #define LV_THEME_DEFAULT_COLOR_PRIMARY      LV_COLOR_BLUE
    #elif defined CONFIG_LVGL_THEME_DEFAULT_PRIMARY_COLOR_NAVY
        #define LV_THEME_DEFAULT_COLOR_PRIMARY      LV_COLOR_NAVY
    #elif defined CONFIG_LVGL_THEME_DEFAULT_PRIMARY_COLOR_MAGENTA
        #define LV_THEME_DEFAULT_COLOR_PRIMARY      LV_COLOR_MAGENTA
    #elif defined CONFIG_LVGL_THEME_DEFAULT_PRIMARY_COLOR_PURPLE
        #define LV_THEME_DEFAULT_COLOR_PRIMARY      LV_COLOR_PURPLE
    #elif defined CONFIG_LVGL_THEME_DEFAULT_PRIMARY_COLOR_ORANGE
        #define LV_THEME_DEFAULT_COLOR_PRIMARY      LV_COLOR_ORANGE
    #else
    #error "Choose valid theme primary color."
    #endif

    /* LV_THEME_DEFAULT_COLOR_SECONDARY */
    #if defined CONFIG_LVGL_THEME_DEFAULT_SECONDARY_COLOR_WHITE
        #define LV_THEME_DEFAULT_COLOR_SECONDARY      LV_COLOR_WHITE
    #elif defined CONFIG_LVGL_THEME_DEFAULT_SECONDARY_COLOR_SILVER
        #define LV_THEME_DEFAULT_COLOR_SECONDARY      LV_COLOR_SILVER
    #elif defined CONFIG_LVGL_THEME_DEFAULT_SECONDARY_COLOR_GRAY
        #define LV_THEME_DEFAULT_COLOR_SECONDARY      LV_COLOR_GRAY
    #elif defined CONFIG_LVGL_THEME_DEFAULT_SECONDARY_COLOR_BLACK
        #define LV_THEME_DEFAULT_COLOR_SECONDARY      LV_COLOR_BLACK
    #elif defined CONFIG_LVGL_THEME_DEFAULT_SECONDARY_COLOR_RED
        #define LV_THEME_DEFAULT_COLOR_SECONDARY      LV_COLOR_RED
    #elif defined CONFIG_LVGL_THEME_DEFAULT_SECONDARY_COLOR_MAROON
        #define LV_THEME_DEFAULT_COLOR_SECONDARY      LV_COLOR_MAROON
    #elif defined CONFIG_LVGL_THEME_DEFAULT_SECONDARY_COLOR_YELLOW
        #define LV_THEME_DEFAULT_COLOR_SECONDARY      LV_COLOR_YELLOW
    #elif defined CONFIG_LVGL_THEME_DEFAULT_SECONDARY_COLOR_OLIVE
        #define LV_THEME_DEFAULT_COLOR_SECONDARY      LV_COLOR_OLIVE
    #elif defined CONFIG_LVGL_THEME_DEFAULT_SECONDARY_COLOR_LIME
        #define LV_THEME_DEFAULT_COLOR_SECONDARY      LV_COLOR_LIME
    #elif defined CONFIG_LVGL_THEME_DEFAULT_SECONDARY_COLOR_GREEN
        #define LV_THEME_DEFAULT_COLOR_SECONDARY      LV_COLOR_GREEN
    #elif defined CONFIG_LVGL_THEME_DEFAULT_SECONDARY_COLOR_CYAN
        #define LV_THEME_DEFAULT_COLOR_SECONDARY      LV_COLOR_CYAN
    #elif defined CONFIG_LVGL_THEME_DEFAULT_SECONDARY_COLOR_AQUA
        #define LV_THEME_DEFAULT_COLOR_SECONDARY      LV_COLOR_AQUA
    #elif defined CONFIG_LVGL_THEME_DEFAULT_SECONDARY_COLOR_TEAL
        #define LV_THEME_DEFAULT_COLOR_SECONDARY      LV_COLOR_TEAL
    #elif defined CONFIG_LVGL_THEME_DEFAULT_SECONDARY_COLOR_BLUE
        #define LV_THEME_DEFAULT_COLOR_SECONDARY      LV_COLOR_BLUE
    #elif defined CONFIG_LVGL_THEME_DEFAULT_SECONDARY_COLOR_NAVY
        #define LV_THEME_DEFAULT_COLOR_SECONDARY      LV_COLOR_NAVY
    #elif defined CONFIG_LVGL_THEME_DEFAULT_SECONDARY_COLOR_MAGENTA
        #define LV_THEME_DEFAULT_COLOR_SECONDARY      LV_COLOR_MAGENTA
    #elif defined CONFIG_LVGL_THEME_DEFAULT_SECONDARY_COLOR_PURPLE
        #define LV_THEME_DEFAULT_COLOR_SECONDARY      LV_COLOR_PURPLE
    #elif defined CONFIG_LVGL_THEME_DEFAULT_SECONDARY_COLOR_ORANGE
        #define LV_THEME_DEFAULT_COLOR_SECONDARY      LV_COLOR_ORANGE
    #else
    #error "Choose valid theme secondary color."
    #endif
#elif defined (CONFIG_LVGL_THEME_MONO)
    #if defined (CONFIG_LVGL_THEME_DEFAULT_COLOR_BLACK)
        #define LV_THEME_DEFAULT_COLOR_PRIMARY      LV_COLOR_BLACK
        #define LV_THEME_DEFAULT_COLOR_SECONDARY    LV_COLOR_WHITE
    #else
        #define LV_THEME_DEFAULT_COLOR_PRIMARY      LV_COLOR_WHITE
        #define LV_THEME_DEFAULT_COLOR_SECONDARY    LV_COLOR_BLACK
    #endif
#endif

#if defined (CONFIG_LVGL_THEME_DEFAULT_INIT_MATERIAL)
#if defined (CONFIG_LVGL_THEME_DEFAULT_FLAG_LIGHT)
    #define LV_THEME_DEFAULT_FLAG               LV_THEME_MATERIAL_FLAG_LIGHT
#elif defined (CONFIG_LVGL_THEME_DEFAULT_FLAG_DARK)
    #define LV_THEME_DEFAULT_FLAG               LV_THEME_MATERIAL_FLAG_DARK
#else
    #define LV_THEME_DEFAULT_FLAG               0
#endif // CONFIG_LVGL_THEME_DEFAULT_FLAG_LIGHT
#else
    #define LV_THEME_DEFAULT_FLAG               0
#endif // CONFIG_LVGL_THEME_DEFAULT_INIT_MATERIAL

#if defined CONFIG_LVGL_FONT_DEFAULT_SMALL_MONTSERRAT_12
#define LV_THEME_DEFAULT_FONT_SMALL         &lv_font_montserrat_12
#elif defined CONFIG_LVGL_FONT_DEFAULT_SMALL_MONTSERRAT_14
#define LV_THEME_DEFAULT_FONT_SMALL         &lv_font_montserrat_14
#elif defined CONFIG_LVGL_FONT_DEFAULT_SMALL_MONTSERRAT_16
#define LV_THEME_DEFAULT_FONT_SMALL         &lv_font_montserrat_16
#elif defined CONFIG_LVGL_FONT_DEFAULT_SMALL_MONTSERRAT_18
#define LV_THEME_DEFAULT_FONT_SMALL         &lv_font_montserrat_18
#elif defined CONFIG_LVGL_FONT_DEFAULT_SMALL_MONTSERRAT_20
#define LV_THEME_DEFAULT_FONT_SMALL         &lv_font_montserrat_20
#elif defined CONFIG_LVGL_FONT_DEFAULT_SMALL_MONTSERRAT_22
#define LV_THEME_DEFAULT_FONT_SMALL         &lv_font_montserrat_22
#elif defined CONFIG_LVGL_FONT_DEFAULT_SMALL_MONTSERRAT_24
#define LV_THEME_DEFAULT_FONT_SMALL         &lv_font_montserrat_24
#elif defined CONFIG_LVGL_FONT_DEFAULT_SMALL_MONTSERRAT_26
#define LV_THEME_DEFAULT_FONT_SMALL         &lv_font_montserrat_26
#elif defined CONFIG_LVGL_FONT_DEFAULT_SMALL_MONTSERRAT_28
#define LV_THEME_DEFAULT_FONT_SMALL         &lv_font_montserrat_28
#elif defined CONFIG_LVGL_FONT_DEFAULT_SMALL_MONTSERRAT_30
#define LV_THEME_DEFAULT_FONT_SMALL         &lv_font_montserrat_30
#elif defined CONFIG_LVGL_FONT_DEFAULT_SMALL_MONTSERRAT_32
#define LV_THEME_DEFAULT_FONT_SMALL         &lv_font_montserrat_32
#elif defined CONFIG_LVGL_FONT_DEFAULT_SMALL_MONTSERRAT_34
#define LV_THEME_DEFAULT_FONT_SMALL         &lv_font_montserrat_34
#elif defined CONFIG_LVGL_FONT_DEFAULT_SMALL_MONTSERRAT_36
#define LV_THEME_DEFAULT_FONT_SMALL         &lv_font_montserrat_36
#elif defined CONFIG_LVGL_FONT_DEFAULT_SMALL_MONTSERRAT_38
#define LV_THEME_DEFAULT_FONT_SMALL         &lv_font_montserrat_38
#elif defined CONFIG_LVGL_FONT_DEFAULT_SMALL_MONTSERRAT_40
#define LV_THEME_DEFAULT_FONT_SMALL         &lv_font_montserrat_40
#elif defined CONFIG_LVGL_FONT_DEFAULT_SMALL_MONTSERRAT_42
#define LV_THEME_DEFAULT_FONT_SMALL         &lv_font_montserrat_42
#elif defined CONFIG_LVGL_FONT_DEFAULT_SMALL_MONTSERRAT_44
#define LV_THEME_DEFAULT_FONT_SMALL         &lv_font_montserrat_44
#elif defined CONFIG_LVGL_FONT_DEFAULT_SMALL_MONTSERRAT_46
#define LV_THEME_DEFAULT_FONT_SMALL         &lv_font_montserrat_46
#elif defined CONFIG_LVGL_FONT_DEFAULT_SMALL_MONTSERRAT_48
#define LV_THEME_DEFAULT_FONT_SMALL         &lv_font_montserrat_48
#elif defined CONFIG_LVGL_FONT_DEFAULT_SMALL_UNSCII8
#define LV_THEME_DEFAULT_FONT_SMALL         &lv_font_unscii_8
#elif defined CONFIG_LVGL_FONT_DEFAULT_SMALL_MONTSERRAT12SUBPIX
#define LV_THEME_DEFAULT_FONT_SMALL         &lv_font_montserrat_12_subpx
#elif defined CONFIG_LVGL_FONT_DEFAULT_SMALL_MONTSERRAT28COMPRESSED
#define LV_THEME_DEFAULT_FONT_SMALL         &lv_font_montserrat_28_compressed
#elif defined CONFIG_LVGL_FONT_DEFAULT_SMALL_DEJAVU_16_PERSIAN_HEBREW
#define LV_THEME_DEFAULT_FONT_SMALL         &lv_font_dejavu_16_persian_hebrew
#elif defined CONFIG_LVGL_FONT_DEFAULT_SMALL_SIMSUN_16_CJK
#define LV_THEME_DEFAULT_FONT_SMALL         &lv_font_simsun_16_cjk
#else /* You can set your custom fonts here */

#endif

#if defined CONFIG_LVGL_FONT_DEFAULT_NORMAL_MONTSERRAT_12
#define LV_THEME_DEFAULT_FONT_NORMAL         &lv_font_montserrat_12
#elif defined CONFIG_LVGL_FONT_DEFAULT_NORMAL_MONTSERRAT_14
#define LV_THEME_DEFAULT_FONT_NORMAL         &lv_font_montserrat_14
#elif defined CONFIG_LVGL_FONT_DEFAULT_NORMAL_MONTSERRAT_16
#define LV_THEME_DEFAULT_FONT_NORMAL         &lv_font_montserrat_16
#elif defined CONFIG_LVGL_FONT_DEFAULT_NORMAL_MONTSERRAT_18
#define LV_THEME_DEFAULT_FONT_NORMAL         &lv_font_montserrat_18
#elif defined CONFIG_LVGL_FONT_DEFAULT_NORMAL_MONTSERRAT_20
#define LV_THEME_DEFAULT_FONT_NORMAL         &lv_font_montserrat_20
#elif defined CONFIG_LVGL_FONT_DEFAULT_NORMAL_MONTSERRAT_22
#define LV_THEME_DEFAULT_FONT_NORMAL         &lv_font_montserrat_22
#elif defined CONFIG_LVGL_FONT_DEFAULT_NORMAL_MONTSERRAT_24
#define LV_THEME_DEFAULT_FONT_NORMAL         &lv_font_montserrat_24
#elif defined CONFIG_LVGL_FONT_DEFAULT_NORMAL_MONTSERRAT_26
#define LV_THEME_DEFAULT_FONT_NORMAL         &lv_font_montserrat_26
#elif defined CONFIG_LVGL_FONT_DEFAULT_NORMAL_MONTSERRAT_28
#define LV_THEME_DEFAULT_FONT_NORMAL         &lv_font_montserrat_28
#elif defined CONFIG_LVGL_FONT_DEFAULT_NORMAL_MONTSERRAT_30
#define LV_THEME_DEFAULT_FONT_NORMAL         &lv_font_montserrat_30
#elif defined CONFIG_LVGL_FONT_DEFAULT_NORMAL_MONTSERRAT_32
#define LV_THEME_DEFAULT_FONT_NORMAL         &lv_font_montserrat_32
#elif defined CONFIG_LVGL_FONT_DEFAULT_NORMAL_MONTSERRAT_34
#define LV_THEME_DEFAULT_FONT_NORMAL         &lv_font_montserrat_34
#elif defined CONFIG_LVGL_FONT_DEFAULT_NORMAL_MONTSERRAT_36
#define LV_THEME_DEFAULT_FONT_NORMAL         &lv_font_montserrat_36
#elif defined CONFIG_LVGL_FONT_DEFAULT_NORMAL_MONTSERRAT_38
#define LV_THEME_DEFAULT_FONT_NORMAL         &lv_font_montserrat_38
#elif defined CONFIG_LVGL_FONT_DEFAULT_NORMAL_MONTSERRAT_40
#define LV_THEME_DEFAULT_FONT_NORMAL         &lv_font_montserrat_40
#elif defined CONFIG_LVGL_FONT_DEFAULT_NORMAL_MONTSERRAT_42
#define LV_THEME_DEFAULT_FONT_NORMAL         &lv_font_montserrat_42
#elif defined CONFIG_LVGL_FONT_DEFAULT_NORMAL_MONTSERRAT_44
#define LV_THEME_DEFAULT_FONT_NORMAL         &lv_font_montserrat_44
#elif defined CONFIG_LVGL_FONT_DEFAULT_NORMAL_MONTSERRAT_46
#define LV_THEME_DEFAULT_FONT_NORMAL         &lv_font_montserrat_46
#elif defined CONFIG_LVGL_FONT_DEFAULT_NORMAL_MONTSERRAT_48
#define LV_THEME_DEFAULT_FONT_NORMAL         &lv_font_montserrat_48
#elif defined CONFIG_LVGL_FONT_DEFAULT_NORMAL_UNSCII8
#define LV_THEME_DEFAULT_FONT_NORMAL         &lv_font_unscii_8
#elif defined CONFIG_LVGL_FONT_DEFAULT_NORMAL_MONTSERRAT12SUBPIX
#define LV_THEME_DEFAULT_FONT_NORMAL         &lv_font_montserrat_12_subpx
#elif defined CONFIG_LVGL_FONT_DEFAULT_NORMAL_MONTSERRAT28COMPRESSED
#define LV_THEME_DEFAULT_FONT_NORMAL         &lv_font_montserrat_28_compressed
#elif defined CONFIG_LVGL_FONT_DEFAULT_NORMAL_DEJAVU_16_PERSIAN_HEBREW
#define LV_THEME_DEFAULT_FONT_NORMAL         &lv_font_dejavu_16_persian_hebrew
#elif defined CONFIG_LVGL_FONT_DEFAULT_NORMAL_SIMSUN_16_CJK
#define LV_THEME_DEFAULT_FONT_NORMAL         &lv_font_simsun_16_cjk
#else /* You can set your custom fonts here */

#endif

#if defined CONFIG_LVGL_FONT_DEFAULT_SUBTITLE_MONTSERRAT_12
#define LV_THEME_DEFAULT_FONT_SUBTITLE         &lv_font_montserrat_12
#elif defined CONFIG_LVGL_FONT_DEFAULT_SUBTITLE_MONTSERRAT_14
#define LV_THEME_DEFAULT_FONT_SUBTITLE         &lv_font_montserrat_14
#elif defined CONFIG_LVGL_FONT_DEFAULT_SUBTITLE_MONTSERRAT_16
#define LV_THEME_DEFAULT_FONT_SUBTITLE         &lv_font_montserrat_16
#elif defined CONFIG_LVGL_FONT_DEFAULT_SUBTITLE_MONTSERRAT_18
#define LV_THEME_DEFAULT_FONT_SUBTITLE         &lv_font_montserrat_18
#elif defined CONFIG_LVGL_FONT_DEFAULT_SUBTITLE_MONTSERRAT_20
#define LV_THEME_DEFAULT_FONT_SUBTITLE         &lv_font_montserrat_20
#elif defined CONFIG_LVGL_FONT_DEFAULT_SUBTITLE_MONTSERRAT_22
#define LV_THEME_DEFAULT_FONT_SUBTITLE         &lv_font_montserrat_22
#elif defined CONFIG_LVGL_FONT_DEFAULT_SUBTITLE_MONTSERRAT_24
#define LV_THEME_DEFAULT_FONT_SUBTITLE         &lv_font_montserrat_24
#elif defined CONFIG_LVGL_FONT_DEFAULT_SUBTITLE_MONTSERRAT_26
#define LV_THEME_DEFAULT_FONT_SUBTITLE         &lv_font_montserrat_26
#elif defined CONFIG_LVGL_FONT_DEFAULT_SUBTITLE_MONTSERRAT_28
#define LV_THEME_DEFAULT_FONT_SUBTITLE         &lv_font_montserrat_28
#elif defined CONFIG_LVGL_FONT_DEFAULT_SUBTITLE_MONTSERRAT_30
#define LV_THEME_DEFAULT_FONT_SUBTITLE         &lv_font_montserrat_30
#elif defined CONFIG_LVGL_FONT_DEFAULT_SUBTITLE_MONTSERRAT_32
#define LV_THEME_DEFAULT_FONT_SUBTITLE         &lv_font_montserrat_32
#elif defined CONFIG_LVGL_FONT_DEFAULT_SUBTITLE_MONTSERRAT_34
#define LV_THEME_DEFAULT_FONT_SUBTITLE         &lv_font_montserrat_34
#elif defined CONFIG_LVGL_FONT_DEFAULT_SUBTITLE_MONTSERRAT_36
#define LV_THEME_DEFAULT_FONT_SUBTITLE         &lv_font_montserrat_36
#elif defined CONFIG_LVGL_FONT_DEFAULT_SUBTITLE_MONTSERRAT_38
#define LV_THEME_DEFAULT_FONT_SUBTITLE         &lv_font_montserrat_38
#elif defined CONFIG_LVGL_FONT_DEFAULT_SUBTITLE_MONTSERRAT_40
#define LV_THEME_DEFAULT_FONT_SUBTITLE         &lv_font_montserrat_40
#elif defined CONFIG_LVGL_FONT_DEFAULT_SUBTITLE_MONTSERRAT_42
#define LV_THEME_DEFAULT_FONT_SUBTITLE         &lv_font_montserrat_42
#elif defined CONFIG_LVGL_FONT_DEFAULT_SUBTITLE_MONTSERRAT_44
#define LV_THEME_DEFAULT_FONT_SUBTITLE         &lv_font_montserrat_44
#elif defined CONFIG_LVGL_FONT_DEFAULT_SUBTITLE_MONTSERRAT_46
#define LV_THEME_DEFAULT_FONT_SUBTITLE         &lv_font_montserrat_46
#elif defined CONFIG_LVGL_FONT_DEFAULT_SUBTITLE_MONTSERRAT_48
#define LV_THEME_DEFAULT_FONT_SUBTITLE         &lv_font_montserrat_48
#elif defined CONFIG_LVGL_FONT_DEFAULT_SUBTITLE_UNSCII8
#define LV_THEME_DEFAULT_FONT_SUBTITLE         &lv_font_unscii_8
#elif defined CONFIG_LVGL_FONT_DEFAULT_SUBTITLE_MONTSERRAT12SUBPIX
#define LV_THEME_DEFAULT_FONT_SUBTITLE         &lv_font_montserrat_12_subpx
#elif defined CONFIG_LVGL_FONT_DEFAULT_SUBTITLE_MONTSERRAT28COMPRESSED
#define LV_THEME_DEFAULT_FONT_SUBTITLE         &lv_font_montserrat_28_compressed
#elif defined CONFIG_LVGL_FONT_DEFAULT_SUBTITLE_DEJAVU_16_PERSIAN_HEBREW
#define LV_THEME_DEFAULT_FONT_SUBTITLE         &lv_font_dejavu_16_persian_hebrew
#elif defined CONFIG_LVGL_FONT_DEFAULT_SUBTITLE_SIMSUN_16_CJK
#define LV_THEME_DEFAULT_FONT_SUBTITLE         &lv_font_simsun_16_cjk
#else /* You can set your custom fonts here */

#endif

#if defined CONFIG_LVGL_FONT_DEFAULT_TITLE_MONTSERRAT_12
#define LV_THEME_DEFAULT_FONT_TITLE         &lv_font_montserrat_12
#elif defined CONFIG_LVGL_FONT_DEFAULT_TITLE_MONTSERRAT_14
#define LV_THEME_DEFAULT_FONT_TITLE         &lv_font_montserrat_14
#elif defined CONFIG_LVGL_FONT_DEFAULT_TITLE_MONTSERRAT_16
#define LV_THEME_DEFAULT_FONT_TITLE         &lv_font_montserrat_16
#elif defined CONFIG_LVGL_FONT_DEFAULT_TITLE_MONTSERRAT_18
#define LV_THEME_DEFAULT_FONT_TITLE         &lv_font_montserrat_18
#elif defined CONFIG_LVGL_FONT_DEFAULT_TITLE_MONTSERRAT_20
#define LV_THEME_DEFAULT_FONT_TITLE         &lv_font_montserrat_20
#elif defined CONFIG_LVGL_FONT_DEFAULT_TITLE_MONTSERRAT_22
#define LV_THEME_DEFAULT_FONT_TITLE         &lv_font_montserrat_22
#elif defined CONFIG_LVGL_FONT_DEFAULT_TITLE_MONTSERRAT_24
#define LV_THEME_DEFAULT_FONT_TITLE         &lv_font_montserrat_24
#elif defined CONFIG_LVGL_FONT_DEFAULT_TITLE_MONTSERRAT_26
#define LV_THEME_DEFAULT_FONT_TITLE         &lv_font_montserrat_26
#elif defined CONFIG_LVGL_FONT_DEFAULT_TITLE_MONTSERRAT_28
#define LV_THEME_DEFAULT_FONT_TITLE         &lv_font_montserrat_28
#elif defined CONFIG_LVGL_FONT_DEFAULT_TITLE_MONTSERRAT_30
#define LV_THEME_DEFAULT_FONT_TITLE         &lv_font_montserrat_30
#elif defined CONFIG_LVGL_FONT_DEFAULT_TITLE_MONTSERRAT_32
#define LV_THEME_DEFAULT_FONT_TITLE         &lv_font_montserrat_32
#elif defined CONFIG_LVGL_FONT_DEFAULT_TITLE_MONTSERRAT_34
#define LV_THEME_DEFAULT_FONT_TITLE         &lv_font_montserrat_34
#elif defined CONFIG_LVGL_FONT_DEFAULT_TITLE_MONTSERRAT_36
#define LV_THEME_DEFAULT_FONT_TITLE         &lv_font_montserrat_36
#elif defined CONFIG_LVGL_FONT_DEFAULT_TITLE_MONTSERRAT_38
#define LV_THEME_DEFAULT_FONT_TITLE         &lv_font_montserrat_38
#elif defined CONFIG_LVGL_FONT_DEFAULT_TITLE_MONTSERRAT_40
#define LV_THEME_DEFAULT_FONT_TITLE         &lv_font_montserrat_40
#elif defined CONFIG_LVGL_FONT_DEFAULT_TITLE_MONTSERRAT_42
#define LV_THEME_DEFAULT_FONT_TITLE         &lv_font_montserrat_42
#elif defined CONFIG_LVGL_FONT_DEFAULT_TITLE_MONTSERRAT_44
#define LV_THEME_DEFAULT_FONT_TITLE         &lv_font_montserrat_44
#elif defined CONFIG_LVGL_FONT_DEFAULT_TITLE_MONTSERRAT_46
#define LV_THEME_DEFAULT_FONT_TITLE         &lv_font_montserrat_46
#elif defined CONFIG_LVGL_FONT_DEFAULT_TITLE_MONTSERRAT_48
#define LV_THEME_DEFAULT_FONT_TITLE         &lv_font_montserrat_48
#elif defined CONFIG_LVGL_FONT_DEFAULT_TITLE_UNSCII8
#define LV_THEME_DEFAULT_FONT_TITLE         &lv_font_unscii_8
#elif defined CONFIG_LVGL_FONT_DEFAULT_TITLE_MONTSERRAT12SUBPIX
#define LV_THEME_DEFAULT_FONT_TITLE         &lv_font_montserrat_12_subpx
#elif defined CONFIG_LVGL_FONT_DEFAULT_TITLE_MONTSERRAT28COMPRESSED
#define LV_THEME_DEFAULT_FONT_TITLE         &lv_font_montserrat_28_compressed
#elif defined CONFIG_LVGL_FONT_DEFAULT_TITLE_DEJAVU_16_PERSIAN_HEBREW
#define LV_THEME_DEFAULT_FONT_TITLE         &lv_font_dejavu_16_persian_hebrew
#elif defined CONFIG_LVGL_FONT_DEFAULT_TITLE_SIMSUN_16_CJK
#define LV_THEME_DEFAULT_FONT_TITLE         &lv_font_simsun_16_cjk
#else /* You can set your custom fonts here */

#endif

/*=================
 *  Text settings
 *=================*/

/* Select a character encoding for strings.
 * Your IDE or editor should have the same character encoding
 * - LV_TXT_ENC_UTF8
 * - LV_TXT_ENC_ASCII
 * */
#if defined CONFIG_LVGL_TXT_ENC_UTF8
#  define LV_TXT_ENC LV_TXT_ENC_UTF8
#elif defined CONFIG_LVGL_TXT_ENC_ASCII
#  define LV_TXT_ENC LV_TXT_ENC_ASCII
#endif

 /*Can break (wrap) texts on these chars*/
#define LV_TXT_BREAK_CHARS                  CONFIG_LVGL_TXT_BREAK_CHARS

/* If a word is at least this long, will break wherever "prettiest"
 * To disable, set to a value <= 0 */
#define LV_TXT_LINE_BREAK_LONG_LEN          CONFIG_LVGL_TXT_LINE_BREAK_LONG_LEN

/* Minimum number of characters in a long word to put on a line before a break.
 * Depends on LV_TXT_LINE_BREAK_LONG_LEN. */
#define LV_TXT_LINE_BREAK_LONG_PRE_MIN_LEN  CONFIG_LVGL_TXT_LINE_BREAK_LONG_PRE_MIN_LEN

/* Minimum number of characters in a long word to put on a line after a break.
 * Depends on LV_TXT_LINE_BREAK_LONG_LEN. */
#define LV_TXT_LINE_BREAK_LONG_POST_MIN_LEN CONFIG_LVGL_TXT_LINE_BREAK_LONG_POST_MIN_LEN

/* The control character to use for signalling text recoloring. */
#define LV_TXT_COLOR_CMD                    CONFIG_LVGL_TXT_COLOR_CMD

/* Support bidirectional texts.
 * Allows mixing Left-to-Right and Right-to-Left texts.
 * The direction will be processed according to the Unicode Bidirectioanl Algorithm:
 * https://www.w3.org/International/articles/inline-bidi-markup/uba-basics*/
#if defined CONFIG_LVGL_BIDI_NO_SUPPORT
#  define LV_USE_BIDI     0
#else
#  define LV_USE_BIDI     1
/* Set the default direction. Supported values:
 * `LV_BIDI_DIR_LTR` Left-to-Right
 * `LV_BIDI_DIR_RTL` Right-to-Left
 * `LV_BIDI_DIR_AUTO` detect texts base direction */
#if defined LVGL_BIDI_DIR_LTR
#  define LV_BIDI_BASE_DIR_DEF      LV_BIDI_DIR_LTR
#elif defined LVGL_BIDI_DIR_RTL 
#  define LV_BIDI_BASE_DIR_DEF      LV_BIDI_DIR_RTL
#elif defined LVGL_BIDI_DIR_AUTO
#  define LV_BIDI_BASE_DIR_DEF      LV_BIDI_DIR_AUTO
#endif
#endif

/* Enable Arabic/Persian processing
 * In these languages characters should be replaced with
 * an other form based on their position in the text */
#if defined CONFIG_LVGL_USE_ARABIC_PERSIAN_CHARS
#  define LV_USE_ARABIC_PERSIAN_CHARS 1
#else
#  define LV_USE_ARABIC_PERSIAN_CHARS 0
#endif

/*Change the built in (v)snprintf functions*/
#if defined CONFIG_LVGL_SPRINTF_CUSTOM
#  define LV_SPRINTF_CUSTOM 1
#else
#  define LV_SPRINTF_CUSTOM 0
#endif

#if LV_SPRINTF_CUSTOM
#  define LV_SPRINTF_INCLUDE <stdio.h>
#  define lv_snprintf     snprintf
#  define lv_vsnprintf    vsnprintf
#else   /*!LV_SPRINTF_CUSTOM*/
#  if defined CONFIG_LVGL_SPRINTF_DISABLE_FLOAT
#    define LV_SPRINTF_DISABLE_FLOAT 1
#  else
#    define LV_SPRINTF_DISABLE_FLOAT 0
#  endif
#endif  /*LV_SPRINTF_CUSTOM*/

/*===================
 *  LV_OBJ SETTINGS
 *==================*/

#if LV_USE_USER_DATA
/*Declare the type of the user data of object (can be e.g. `void *`, `int`, `struct`)*/
typedef void * lv_obj_user_data_t;
/*Provide a function to free user data*/
#define LV_USE_USER_DATA_FREE 0
#if LV_USE_USER_DATA_FREE
#  define LV_USER_DATA_FREE_INCLUDE  "something.h"  /*Header for user data free function*/
/* Function prototype : void user_data_free(lv_obj_t * obj); */
#  define LV_USER_DATA_FREE  (user_data_free)       /*Invoking for user data free function*/
#endif
#endif

/*1: enable `lv_obj_realaign()` based on `lv_obj_align()` parameters*/
#define LV_USE_OBJ_REALIGN          1

/* Enable to make the object clickable on a larger area.
 * LV_EXT_CLICK_AREA_OFF or 0: Disable this feature
 * LV_EXT_CLICK_AREA_TINY: The extra area can be adjusted horizontally and vertically (0..255 px)
 * LV_EXT_CLICK_AREA_FULL: The extra area can be adjusted in all 4 directions (-32k..+32k px)
 */
#define LV_USE_EXT_CLICK_AREA  LV_EXT_CLICK_AREA_TINY

/*==================
 *  LV OBJ X USAGE
 *================*/
/*
 * Documentation of the object types: https://docs.lvgl.com/#Object-types
 */

/*Arc (dependencies: -)*/
#if defined (CONFIG_LVGL_WIDGETS_USE_ARC)
    #define LV_USE_ARC      1
#else
    #define LV_USE_ARC      0 
#endif

/*Bar (dependencies: -)*/
#if defined (CONFIG_LVGL_WIDGETS_USE_BAR)
    #define LV_USE_BAR      1
#else
    #define LV_USE_BAR      0
#endif

/*Button (dependencies: lv_cont*/
#if defined (CONFIG_LVGL_WIDGETS_USE_BTN)
    #define LV_USE_BTN      1
#else
    #define LV_USE_BTN      0
#endif

/*Button matrix (dependencies: -)*/
#if defined (CONFIG_LVGL_WIDGETS_USE_BTNMATRIX)
    #define LV_USE_BTNMATRIX    1
#else
    #define LV_USE_BTNMATRIX    0
#endif

/*Calendar (dependencies: -)*/
#if defined (CONFIG_LVGL_WIDGETS_USE_CALENDAR)
    #define LV_USE_CALENDAR     1
#else
    #define LV_USE_CALENDAR     0
#endif

/*Canvas (dependencies: lv_img)*/
#if defined (CONFIG_LVGL_WIDGETS_USE_CANVAS)
    #define LV_USE_CANVAS       1
#else
    #define LV_USE_CANVAS       0
#endif

/*Check box (dependencies: lv_btn, lv_label)*/
#if defined (CONFIG_LVGL_WIDGETS_USE_CHECKBOX)
    #define LV_USE_CHECKBOX         1
#else
    #define LV_USE_CHECKBOX         0
#endif

/*Chart (dependencies: -)*/
#if defined (CONFIG_LVGL_WIDGETS_USE_CHART)
    #define LV_USE_CHART            1
#else
    #define LV_USE_CHART            0
#endif

#if LV_USE_CHART
#  define LV_CHART_AXIS_TICK_LABEL_MAX_LEN    CONFIG_LVGL_WIDGETS_CHART_AXIS_MAX_LEN
#endif

/*Container (dependencies: -*/
#if defined (CONFIG_LVGL_WIDGETS_USE_CONTAINER)
    #define LV_USE_CONT             1
#else
    #define LV_USE_CONT             0
#endif

/*Color picker (dependencies: -*/
#if defined (CONFIG_LVGL_WIDGETS_USE_CPICKER)
    #define LV_USE_CPICKER          1
#else
    #define LV_USE_CPICKER          0
#endif

/*Drop down list (dependencies: lv_page, lv_label, lv_symbol_def.h)*/
#if defined (CONFIG_LVGL_WIDGETS_USE_DROPDOWN)
    #define LV_USE_DROPDOWN     1
#else
    #define LV_USE_DROPDOWN     0
#endif

#if LV_USE_DROPDOWN != 0
/*Open and close default animation time [ms] (0: no animation)*/
#  define LV_DROPDOWN_DEF_ANIM_TIME CONFIG_LVGL_WIDGETS_DROPDOWN_ANIMATION_TIME
#endif

/*Gauge (dependencies:lv_bar, lv_linemeter)*/
#if defined (CONFIG_LVGL_WIDGETS_USE_GAUGE)
    #define LV_USE_GAUGE            1
#else
    #define LV_USE_GAUGE            0
#endif

/*Image (dependencies: lv_label*/
#if defined (CONFIG_LVGL_WIDGETS_USE_IMG)
    #define LV_USE_IMG              1
#else
    #define LV_USE_IMG              0
#endif

/*Image Button (dependencies: lv_btn*/
#if defined (CONFIG_LVGL_WIDGETS_USE_IMGBTN)
    #define LV_USE_IMGBTN           1
#else
    #define LV_USE_IMGBTN           0
#endif

#if LV_USE_IMGBTN
/*1: The imgbtn requires left, mid and right parts and the width can be set freely*/
#  define LV_IMGBTN_TILED 0
#endif

/*Keyboard (dependencies: lv_btnm)*/
#if defined (CONFIG_LVGL_WIDGETS_USE_KEYBOARD)
    #define LV_USE_KEYBOARD         1
#else
    #define LV_USE_KEYBOARD         0
#endif

/*Label (dependencies: -*/
#if defined (CONFIG_LVGL_WIDGETS_USE_LABEL)
    #define LV_USE_LABEL            1
#else
    #define LV_USE_LABEL            0
#endif

#if LV_USE_LABEL != 0
/*Hor, or ver. scroll speed [px/sec] in 'LV_LABEL_LONG_ROLL/ROLL_CIRC' mode*/
    #define LV_LABEL_DEF_SCROLL_SPEED       CONFIG_LVGL_WIDGETS_LABEL_DEF_SCROLL_SPEED
/* Waiting period at beginning/end of animation cycle */
    #define LV_LABEL_WAIT_CHAR_COUNT        CONFIG_LVGL_WIDGETS_LABEL_WAIT_CHAR_COUNT
/*Enable selecting text of the label */
    #define LV_LABEL_TEXT_SEL               0
/*Store extra some info in labels (12 bytes) to speed up drawing of very long texts*/
    #define LV_LABEL_LONG_TXT_HINT          0
#endif

/*LED (dependencies: -)*/
#if defined (CONFIG_LVGL_WIDGETS_USE_LED)
    #define LV_USE_LED              1
#else
    #define LV_USE_LED              0
#endif

#if LV_USE_LED
#  define LV_LED_BRIGHT_MIN CONFIG_LVGL_WIDGETS_LED_BRIGHT_MIN /*Minimal brightness*/
#  define LV_LED_BRIGHT_MAX CONFIG_LVGL_WIDGETS_LED_BRIGHT_MAX /*Maximal brightness*/
#endif

/*Line (dependencies: -*/
#if defined (CONFIG_LVGL_WIDGETS_USE_LINE)
    #define LV_USE_LINE             1
#else
    #define LV_USE_LINE             0
#endif

/*List (dependencies: lv_page, lv_btn, lv_label, (lv_img optionally for icons ))*/
#if defined (CONFIG_LVGL_WIDGETS_USE_LIST)
    #define LV_USE_LIST             1
#else
    #define LV_USE_LIST             0
#endif

#if LV_USE_LIST != 0
/*Default animation time of focusing to a list element [ms] (0: no animation)  */
#  define LV_LIST_DEF_ANIM_TIME  CONFIG_LVGL_WIDGETS_LIST_DEFAULT_ANIMATION_TIME
#endif

/*Line meter (dependencies: *;)*/
#if defined (CONFIG_LVGL_WIDGETS_USE_LINEMETER)
    #define LV_USE_LINEMETER        1
#else
    #define LV_USE_LINEMETER        0
#endif

#if LV_USE_LINEMETER
/* Draw line more precisely at cost of performance.
 * Useful if there are lot of lines any minor are visible
 * 0: No extra precision
 * 1: Some extra precision
 * 2: Best precision
 */
#  define LV_LINEMETER_PRECISE    0
#endif

/*Mask (dependencies: -)*/
#if defined (CONFIG_LVGL_WIDGETS_USE_OBJMASK)
    #define LV_USE_OBJMASK          1
#else
    #define LV_USE_OBJMASK          0
#endif

/*Message box (dependencies: lv_rect, lv_btnm, lv_label)*/
#if defined (CONFIG_LVGL_WIDGETS_USE_MSGBOX)
    #define LV_USE_MSGBOX           1
#else
    #define LV_USE_MSGBOX           0
#endif

/*Page (dependencies: lv_cont)*/
#if defined (CONFIG_LVGL_WIDGETS_USE_PAGE)
    #define LV_USE_PAGE             1
#else
    #define LV_USE_PAGE             0
#endif

#if LV_USE_PAGE != 0
/*Focus default animation time [ms] (0: no animation)*/
#  define LV_PAGE_DEF_ANIM_TIME CONFIG_LVGL_WIDGETS_PAGE_ANIMATION_DEFAULT_TIME
#endif

/*Preload (dependencies: lv_arc, lv_anim)*/
#if defined (CONFIG_LVGL_WIDGETS_USE_SPINNER)
    #define LV_USE_SPINNER          1
#else
    #define LV_USE_SPINNER          0
#endif

#if LV_USE_SPINNER != 0
    #define LV_SPINNER_DEF_ARC_LENGTH   CONFIG_LVGL_WIDGETS_SPINNER_DEF_ARC_LENGTH /*[deg]*/
    #define LV_SPINNER_DEF_SPIN_TIME    CONFIG_LVGL_WIDGETS_SPINNER_DEF_SPIN_TIME /*[ms]*/
    #define LV_SPINNER_DEF_ANIM         LV_SPINNER_TYPE_SPINNING_ARC
#endif

/*Roller (dependencies: lv_ddlist)*/
#if defined (CONFIG_LVGL_WIDGETS_USE_ROLLER)
    #define LV_USE_ROLLER           1
#else
    #define LV_USE_ROLLER           0
#endif

#if LV_USE_ROLLER != 0
/*Focus animation time [ms] (0: no animation)*/
    #define LV_ROLLER_DEF_ANIM_TIME     CONFIG_LVGL_WIDGETS_ROLLER_DEF_ANIM_TIME
/*Number of extra "pages" when the roller is infinite*/
    #define LV_ROLLER_INF_PAGES         CONFIG_LVGL_WIDGETS_ROLLER_INF_PAGES
#endif

/*Slider (dependencies: lv_bar)*/
#if defined (CONFIG_LVGL_WIDGETS_USE_SLIDER)
    #define LV_USE_SLIDER           1
#else
    #define LV_USE_SLIDER           0
#endif

/*Spinbox (dependencies: lv_ta)*/
#if defined (CONFIG_LVGL_WIDGETS_USE_SPINBOX)
    #define LV_USE_SPINBOX          1
#else
    #define LV_USE_SPINBOX          0
#endif

/*Switch (dependencies: lv_slider)*/
#if defined (CONFIG_LVGL_WIDGETS_USE_SWITCH)
    #define LV_USE_SWITCH           1
#else
    #define LV_USE_SWITCH           0
#endif

/*Text area (dependencies: lv_label, lv_page)*/
#if defined (CONFIG_LVGL_WIDGETS_USE_TEXTAREA)
    #define LV_USE_TEXTAREA         1
#else
    #define LV_USE_TEXTAREA         0
#endif

#if LV_USE_TEXTAREA != 0
    #define LV_TEXTAREA_DEF_CURSOR_BLINK_TIME CONFIG_LVGL_WIDGETS_TEXTAREA_DEF_CURSOR_BLINK_TIME     /*ms*/
    #define LV_TEXTAREA_DEF_PWD_SHOW_TIME     CONFIG_LVGL_WIDGETS_TEXTAREA_DEF_PWN_SHOW_TIME    /*ms*/
#endif

/*Table (dependencies: lv_label)*/
#if defined (CONFIG_LVGL_WIDGETS_USE_TABLE)
    #define LV_USE_TABLE            1
#else
    #define LV_USE_TABLE            0
#endif

#if LV_USE_TABLE
    #define LV_TABLE_COL_MAX    CONFIG_LVGL_WIDGETS_TABLE_COL_MAX
#endif

/*Tab (dependencies: lv_page, lv_btnm)*/
#if defined (CONFIG_LVGL_WIDGETS_USE_TABVIEW)
    #define LV_USE_TABVIEW          1
#else
    #define LV_USE_TABVIEW          0
#endif

#if LV_USE_TABVIEW != 0
/*Time of slide animation [ms] (0: no animation)*/
    #define LV_TABVIEW_DEF_ANIM_TIME    CONFIG_LVGL_WIDGETS_TABVIEW_SLIDE_ANIMATION 
#endif

/*Tileview (dependencies: lv_page) */
#if defined (CONFIG_LVGL_WIDGETS_USE_TILEVIEW)
    #define LV_USE_TILEVIEW     1
#else
    #define LV_USE_TILEVIEW     0
#endif

#if LV_USE_TILEVIEW
/*Time of slide animation [ms] (0: no animation)*/
    #define LV_TILEVIEW_DEF_ANIM_TIME   CONFIG_LVGL_WIDGETS_TILEVIEW_SLIDE_ANIMATION
#endif

/*Window (dependencies: lv_cont, lv_btn, lv_label, lv_img, lv_page)*/
#if defined (CONFIG_LVGL_WIDGETS_USE_WINDOW)
    #define LV_USE_WIN  1
#else
    #define LV_USE_WIN  0
#endif

/*==================
 * Non-user section
 *==================*/

#if defined(_MSC_VER) && !defined(_CRT_SECURE_NO_WARNINGS)    /* Disable warnings for Visual Studio*/
#  define _CRT_SECURE_NO_WARNINGS
#endif

/*--END OF LV_CONF_H--*/

#endif /*LV_CONF_H*/

#endif /*End of "Content enable"*/
