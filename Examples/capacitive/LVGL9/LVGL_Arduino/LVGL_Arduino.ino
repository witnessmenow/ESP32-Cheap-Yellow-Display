/* Using LVGL with Arduino requires some extra steps...
 *  
 * Be sure to read the docs here: https://docs.lvgl.io/master/integration/framework/arduino.html
 * but note you should use the lv_conf.h from the repo as it is pre-edited to work.
 * 
 * You can always edit your own lv_conf.h later and exclude the example options once the build environment is working.
 * 
 * Note you MUST move the 'examples' and 'demos' folders into the 'src' folder inside the lvgl library folder 
 * otherwise this will not compile, please see README.md in the repo.
 * 
 */
#include <lvgl.h>

#include <TFT_eSPI.h>

#include <examples/lv_examples.h>
#include <demos/lv_demos.h>

#include <bb_captouch.h>
// A library for interfacing with the touch screen
//
// Can be installed from the library manager (Search for "bb_captouch")
uint16_t touchScreenMinimumX = 10, touchScreenMaximumX = 470, touchScreenMinimumY = 10,touchScreenMaximumY = 310;

/*Set to your screen resolution*/
#define TFT_HOR_RES   480
#define TFT_VER_RES   320

// Pins to use for touch on ESP32 LCD CYD with capacitive GT911 touch controller
#define TOUCH_SDA 33
#define TOUCH_SCL 32
#define TOUCH_INT 21
#define TOUCH_RST 25

/*LVGL draw into this buffer, 1/10 screen size usually works well. The size is in bytes*/
#define DRAW_BUF_SIZE (TFT_HOR_RES * TFT_VER_RES / 10 * (LV_COLOR_DEPTH / 8))

BBCapTouch bbct;

TFT_eSPI tft = TFT_eSPI(TFT_HOR_RES, TFT_VER_RES);

#if LV_USE_LOG != 0
void my_print( lv_log_level_t level, const char * buf )
{
    LV_UNUSED(level);
    Serial.println(buf);
    Serial.flush();
}
#endif

/* LVGL calls it when a rendered image needs to copied to the display*/
void my_disp_flush( lv_display_t *disp, const lv_area_t *area, uint8_t * px_map)
{
    /*Call it to tell LVGL you are ready*/
    lv_disp_flush_ready(disp);
}
/*Read the touchpad*/
void my_touchpad_read( lv_indev_t * indev, lv_indev_data_t * data )
{
  TOUCHINFO ti;

  if(bbct.getSamples(&ti))
  {
    // Some very basic auto calibration so it doesn't go out of range
    if(ti.y[0] < touchScreenMinimumX) touchScreenMinimumX = ti.y[0];
    if(ti.y[0] > touchScreenMaximumX) touchScreenMaximumX = ti.y[0];
    if(ti.x[0] < touchScreenMinimumY) touchScreenMinimumY = ti.x[0];
    if(ti.x[0] > touchScreenMaximumY) touchScreenMaximumY = ti.x[0];

    // Map the touched position to the pixel position, ti.y[i], ti.area[i]
    data->point.x = map(TFT_HOR_RES-ti.y[0],touchScreenMinimumX,touchScreenMaximumX,1,TFT_HOR_RES); /* Touchscreen X calibration */
    data->point.y = map(ti.x[0],touchScreenMinimumY,touchScreenMaximumY,1,TFT_VER_RES); /* Touchscreen Y calibration */
    data->state = LV_INDEV_STATE_PRESSED;
    
    Serial.print("Touch x ");
    Serial.print(data->point.x);
    Serial.print(" y ");
    Serial.println(data->point.y);
    
  }
  else
  {
    data->state = LV_INDEV_STATE_RELEASED;
  }
}

lv_indev_t * indev; //Touchscreen input device
uint8_t* draw_buf;  //draw_buf is allocated on heap otherwise the static area is too big on ESP32 at compile
uint32_t lastTick = 0;  //Used to track the tick timer

void setup()
{
  // Start the tft display and set it to black
  tft.init();
  tft.setRotation(3); //This is the display in landscape

  //Some basic info on the Serial console
  String LVGL_Arduino = "LVGL demo ";
  LVGL_Arduino += String('V') + lv_version_major() + "." + lv_version_minor() + "." + lv_version_patch();
  Serial.begin(115200);
  Serial.println(LVGL_Arduino);
    
  // Initialise the touchscreen
  bbct.init(TOUCH_SDA, TOUCH_SCL, TOUCH_RST, TOUCH_INT);

  //Initialise LVGL
  lv_init();
  draw_buf = new uint8_t[DRAW_BUF_SIZE];
  lv_display_t * disp;
  disp = lv_tft_espi_create(TFT_HOR_RES, TFT_VER_RES, draw_buf, DRAW_BUF_SIZE);

  indev = lv_indev_create();
  lv_indev_set_type(indev, LV_INDEV_TYPE_POINTER);  
  lv_indev_set_read_cb(indev, my_touchpad_read);

  /* Try an example. See all the examples 
   * online: https://docs.lvgl.io/master/examples.html
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
  
  //Or try out the large standard widgets demo
  lv_demo_widgets();
  // lv_demo_benchmark();          
  // lv_demo_keypad_encoder();     

  //Done
  Serial.println( "Setup done" );
}

void loop()
{   
    lv_tick_inc(millis() - lastTick); //Update the tick timer. Tick is new for LVGL 9
    lastTick = millis();
    lv_timer_handler();               //Update the UI
    delay(5);
}
