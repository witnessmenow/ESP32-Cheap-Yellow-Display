/*Using LVGL with Arduino requires some extra steps:
 *Be sure to read the docs here: https://docs.lvgl.io/8.3/get-started/platforms/arduino.html  */

#include <lvgl.h>
#include <TFT_eSPI.h>
#include "examples/lv_examples.h"
#include "demos/lv_demos.h"

#include <XPT2046_Touchscreen.h>
// A library for interfacing with the touch screen
//
// Can be installed from the library manager (Search for "XPT2046")
//https://github.com/PaulStoffregen/XPT2046_Touchscreen
// ----------------------------
// Touch Screen pins
// ----------------------------

// The CYD touch uses some non default
// SPI pins

#define XPT2046_IRQ 36
#define XPT2046_MOSI 32
#define XPT2046_MISO 39
#define XPT2046_CLK 25
#define XPT2046_CS 33
SPIClass mySpi = SPIClass(HSPI);
XPT2046_Touchscreen ts(XPT2046_CS, XPT2046_IRQ);


/*Change to your screen resolution*/
static const uint16_t screenWidth  = 320;
static const uint16_t screenHeight = 240;

static lv_disp_draw_buf_t draw_buf;
static lv_color_t buf[ screenWidth * screenHeight / 10 ];

TFT_eSPI tft = TFT_eSPI(screenWidth, screenHeight); /* TFT instance */

#if LV_USE_LOG != 0
/* Serial debugging */
void my_print(const char * buf)
{
    Serial.printf(buf);
    Serial.flush();
}
#endif

/* Display flushing */
void my_disp_flush( lv_disp_drv_t *disp_drv, const lv_area_t *area, lv_color_t *color_p )
{
    uint32_t w = ( area->x2 - area->x1 + 1 );
    uint32_t h = ( area->y2 - area->y1 + 1 );

    tft.startWrite();
    tft.setAddrWindow( area->x1, area->y1, w, h );
    tft.pushColors( ( uint16_t * )&color_p->full, w * h, true );
    tft.endWrite();

    lv_disp_flush_ready( disp_drv );
}

/*Read the touchpad*/
void my_touchpad_read( lv_indev_drv_t * indev_drv, lv_indev_data_t * data )
{
    uint16_t touchX, touchY;

    //bool touched = tft.getTouch( &touchX, &touchY, 600 );
    //bool touched = false;
    bool touched = (ts.tirqTouched() && ts.touched());

    if( !touched )
    {
        data->state = LV_INDEV_STATE_REL;
    }
    else
    {
        TS_Point p = ts.getPoint();
        touchX = map(p.x,200,3700,1,screenWidth); /* Touchscreen X calibration */
        touchY = map(p.y,240,3800,1,screenHeight); /* Touchscreen X calibration */
        data->state = LV_INDEV_STATE_PR;

        /*Set the coordinates*/
        data->point.x = touchX;
        data->point.y = touchY;

        Serial.print( "Data x " );
        Serial.println( touchX );

        Serial.print( "Data y " );
        Serial.println( touchY );
    }
}

void setup()
{
    Serial.begin( 115200 ); /* prepare for possible serial debug */

    String LVGL_Arduino = "Hello Arduino! ";
    LVGL_Arduino += String('V') + lv_version_major() + "." + lv_version_minor() + "." + lv_version_patch();

    Serial.println( LVGL_Arduino );
    Serial.println( "I am LVGL_Arduino" );

    lv_init();

#if LV_USE_LOG != 0
    lv_log_register_print_cb( my_print ); /* register print function for debugging */
#endif

    mySpi.begin(XPT2046_CLK, XPT2046_MISO, XPT2046_MOSI, XPT2046_CS); /* Start second SPI bus for touchscreen */
    ts.begin(mySpi); /* Touchscreen init */
    ts.setRotation(1); /* Landscape orientation */

    tft.begin();          /* TFT init */
    tft.setRotation( 1 ); /* Landscape orientation */

    lv_disp_draw_buf_init( &draw_buf, buf, NULL, screenWidth * screenHeight / 10 );

    /*Initialize the display*/
    static lv_disp_drv_t disp_drv;
    lv_disp_drv_init( &disp_drv );
    /*Change the following line to your display resolution*/
    disp_drv.hor_res = screenWidth;
    disp_drv.ver_res = screenHeight;
    disp_drv.flush_cb = my_disp_flush;
    disp_drv.draw_buf = &draw_buf;
    lv_disp_drv_register( &disp_drv );

    /*Initialize the (dummy) input device driver*/
    static lv_indev_drv_t indev_drv;
    lv_indev_drv_init( &indev_drv );
    indev_drv.type = LV_INDEV_TYPE_POINTER;
    indev_drv.read_cb = my_touchpad_read;
    lv_indev_drv_register( &indev_drv );
  
    /* Uncomment to create simple label */
    // lv_obj_t *label = lv_label_create( lv_scr_act() );
    // lv_label_set_text( label, "Hello Ardino and LVGL!");
    // lv_obj_align( label, LV_ALIGN_CENTER, 0, 0 );
 
    /* Try an example. See all the examples 
     * online: https://docs.lvgl.io/master/examples.html
     * source codes: https://github.com/lvgl/lvgl/tree/e7f88efa5853128bf871dde335c0ca8da9eb7731/examples
     */
     // lv_example_animimg_1();
     // lv_example_arc_1();
     // lv_example_arc_2();
     // lv_example_bar_1();
     // lv_example_bar_2();
     // lv_example_bar_3();
     // lv_example_bar_4();
     // lv_example_bar_5();
     // lv_example_bar_6();
     // lv_example_btn_1();
     // lv_example_btn_2();
     // lv_example_btn_3();
     // lv_example_btnmatrix_1();
     // lv_example_btnmatrix_2();
     // lv_example_btnmatrix_3();
     // lv_example_calendar_1();
     // lv_example_chart_1();
     // lv_example_chart_2();
     // lv_example_chart_3();
     // lv_example_chart_4();
     // lv_example_chart_5();
     // lv_example_chart_6();
     // lv_example_chart_7();
     // lv_example_chart_8();
     // lv_example_chart_9();
     // lv_example_checkbox_1();
     // lv_example_checkbox_2();
     // lv_example_dropdown_1();
     // lv_example_dropdown_2();
     // lv_example_dropdown_3();
     // lv_example_keyboard_1();
     // lv_example_label_1();
     // lv_example_label_2();
     // lv_example_label_3();
     // lv_example_label_4();
     // lv_example_label_5();
     // lv_example_line_1();
     // lv_example_list_1();
     // lv_example_list_2();
     // lv_example_msgbox_1();
     // lv_example_roller_1();
     // lv_example_roller_2();
     // lv_example_roller_3();
     // lv_example_slider_1();
     // lv_example_slider_2();
     // lv_example_slider_3();
     // lv_example_span_1();
     // lv_example_spinbox_1();
     // lv_example_spinner_1();
     // lv_example_switch_1();
     // lv_example_table_1();
     // lv_example_table_2();
     // lv_example_tabview_1();
     // lv_example_tabview_2();
     // lv_example_textarea_1();
     // lv_example_textarea_2();
     // lv_example_textarea_3();
     // lv_example_tileview_1();
     // lv_example_win_1();
   
     /*Or try out a demo. Don't forget to enable the demos in lv_conf.h. E.g. LV_USE_DEMO_WIDGETS*/
    lv_demo_widgets();               
    // lv_demo_benchmark();          
    // lv_demo_keypad_encoder();     
    
    Serial.println( "Setup done" );
}

void loop()
{
    lv_timer_handler(); /* let the GUI do its work */
    delay( 5 );
}
