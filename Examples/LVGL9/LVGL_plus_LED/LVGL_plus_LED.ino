// this demo features three LVGL sliders used to control the built-in LEDs on the CYD
// if you have trouble running this demo, reffer to Examples/LVGL9/LVGL_Arduino/LVGL_Arduino.ino
// if you can succesfully run the LVGL9 demo, this one should run with no additional setup

#include <lvgl.h>
#include <TFT_eSPI.h>
#include <XPT2046_Touchscreen.h>

#define TFT_HOR_RES   320
#define TFT_VER_RES   240
#define DRAW_BUF_SIZE (TFT_HOR_RES * TFT_VER_RES / 10 * (LV_COLOR_DEPTH / 8))

// touchscreen and LVGL global setup
#define XPT2046_IRQ 36
#define XPT2046_MOSI 32
#define XPT2046_MISO 39
#define XPT2046_CLK 25
#define XPT2046_CS 33

SPIClass touchscreenSpi = SPIClass(VSPI);
XPT2046_Touchscreen touchscreen(XPT2046_CS, XPT2046_IRQ);

uint16_t touchScreenMinimumX = 200, touchScreenMaximumX = 3700, touchScreenMinimumY = 240, touchScreenMaximumY = 3800;
uint32_t lastTick = 0;

void my_touchpad_read(lv_indev_t * indev, lv_indev_data_t * data) {
    if (touchscreen.touched()) {
        TS_Point p = touchscreen.getPoint();
        data->point.x = map(p.x, touchScreenMinimumX, touchScreenMaximumX, 0, TFT_HOR_RES);
        data->point.y = map(p.y, touchScreenMinimumY, touchScreenMaximumY, 0, TFT_VER_RES);
        data->state = LV_INDEV_STATE_PRESSED;
    } else {
        data->state = LV_INDEV_STATE_RELEASED;
    }
}

// onboard LEDs
#define CYD_LED_BLUE 17
#define CYD_LED_RED 4
#define CYD_LED_GREEN 16

// values used to reduce the power of the individual R, G and B LEDs
// the green one is very weak compared to the other ones
// with this, the colors mix a lot better
#define RED_FILTER 0.25
#define GREEN_FILTER 1
#define BLUE_FILTER 0.4

// global variable to store the color selected by the sliders - it is used both for the color of the color of the sliders and the color of the integrated LED
lv_color_t sliderColor = lv_color_make(255, 255, 255);

// declaration of the callback function passed to the LVGL sliders - it will get called when they are interacted with
static void slider_event_cb(lv_event_t *e);

// global arrayw used to store the refference to the LVGL elemets, used to change their value later
lv_obj_t * sliders[3];
lv_obj_t * labels[3];

// function used to place the LVGL elements onto the screens
void setup_ui(lv_obj_t * parent) {
    // create the first slider and store it's refference for later use
    sliders[0] = lv_slider_create(parent);

    // set the width of the slider
    lv_obj_set_width(sliders[0], 220);

    // set the slider's position
    lv_obj_align(sliders[0], LV_ALIGN_TOP_MID, 0, 40);

    // set the range of values produced by the slider
    lv_slider_set_range(sliders[0], 0, 255);

    // set the default value
    lv_slider_set_value(sliders[0], 255, LV_ANIM_OFF);

    // set the color of he knob, the track (background line) and the indicator (selected value line)
    apply_slider_styles(sliders[0]);

    // set the callback called when the value of this slider changes
    lv_obj_add_event_cb(sliders[0], slider_event_cb, LV_EVENT_VALUE_CHANGED, NULL);

    // repeat for the second sliders
    sliders[1] = lv_slider_create(parent);
    lv_obj_set_width(sliders[1], 220);
    lv_obj_align(sliders[1], LV_ALIGN_TOP_MID, 0, 90);
    lv_slider_set_range(sliders[1], 0, 255);
    lv_slider_set_value(sliders[1], 255, LV_ANIM_OFF);
    apply_slider_styles(sliders[1]);
    lv_obj_add_event_cb(sliders[1], slider_event_cb, LV_EVENT_VALUE_CHANGED, NULL);


    // repeat for the third slider
    sliders[2] = lv_slider_create(parent);
    lv_obj_set_width(sliders[2], 220);
    lv_obj_align(sliders[2], LV_ALIGN_TOP_MID, 0, 140);
    lv_slider_set_range(sliders[2], 0, 255);
    lv_slider_set_value(sliders[2], 255, LV_ANIM_OFF);
    apply_slider_styles(sliders[2]);
    lv_obj_add_event_cb(sliders[2], slider_event_cb, LV_EVENT_VALUE_CHANGED, NULL);

    // create a style for the labels
    static lv_style_t style;
    lv_style_init(&style);
    lv_style_set_text_color(&style, lv_color_white());  // Set text color to white

    // rerender the first value label
    labels[0] = lv_label_create(parent);
    lv_obj_set_width(labels[0], 42);
    lv_obj_align(labels[0], LV_ALIGN_BOTTOM_LEFT, 50, -30);

    // set label text
    lv_label_set_text(labels[0], "R: 255");

    // apply the style
    lv_obj_add_style(labels[0], &style, 0);


    // repeat for other labels
    labels[1] = lv_label_create(parent);
    lv_obj_set_width(labels[1], 42);
    lv_obj_align(labels[1], LV_ALIGN_BOTTOM_MID, 0, -30);
    lv_label_set_text(labels[1], "G: 255");
    lv_obj_add_style(labels[1], &style, 0);

    labels[2] = lv_label_create(parent);
    lv_obj_set_width(labels[2], 42);
    lv_obj_align(labels[2], LV_ALIGN_BOTTOM_RIGHT, -50, -30);
    lv_label_set_text(labels[2], "B: 255");
    lv_obj_add_style(labels[2], &style, 0);
}

// define the callback function.
static void slider_event_cb(lv_event_t * e)
{
  // get the values of the sliders using the refferences within the gloabal array
  int red = lv_slider_get_value(sliders[0]);
  int green = lv_slider_get_value(sliders[1]);
  int blue = lv_slider_get_value(sliders[2]);

  // set the global slider color variable's value.
  sliderColor = lv_color_make(red, green, blue);

  // update the sliders with this value to display the color on-screen as well
  apply_slider_styles(sliders[0]);
  apply_slider_styles(sliders[1]);
  apply_slider_styles(sliders[2]);

  // update the labels with the new values
  update_label_text(labels[0], 'R', red);
  update_label_text(labels[1], 'G', green);
  update_label_text(labels[2], 'B', blue);
}

// helper function to apply styles
void apply_slider_styles(lv_obj_t* slider) {
    lv_obj_set_style_bg_color(slider, sliderColor, LV_PART_MAIN);  // set the track color
    lv_obj_set_style_bg_color(slider, sliderColor, LV_PART_INDICATOR); // set the indicator color
    lv_obj_set_style_bg_color(slider, lv_color_lighten(sliderColor, LV_OPA_30), LV_PART_KNOB); // set the knob color
};

// helper function to update label text
void update_label_text(lv_obj_t* label, char symbol, int number){
  char buf[128];
  snprintf(buf, sizeof(buf), "%c: %d",symbol, number); // generate the text value with the provided character and number
  lv_label_set_text(label, buf); // apply the text to the label
}

void setup() {
    Serial.begin(115200);

    lv_init(); // initialize LVGL

    touchscreenSpi.begin(XPT2046_CLK, XPT2046_MISO, XPT2046_MOSI, XPT2046_CS); // start second SPI bus for touchscreen
    touchscreen.begin(touchscreenSpi); // touchscreen init
    touchscreen.setRotation(3); // adjust as necessary
    
    uint8_t* draw_buf = new uint8_t[DRAW_BUF_SIZE];
    lv_display_t * disp = lv_tft_espi_create(TFT_HOR_RES, TFT_VER_RES, draw_buf, DRAW_BUF_SIZE);

    lv_obj_t *scr = lv_scr_act(); // get the active screen object
    lv_obj_set_style_bg_color(scr, lv_color_black(), 0); // set the background of the active screen object to black

    lv_indev_t * indev = lv_indev_create();
    lv_indev_set_type(indev, LV_INDEV_TYPE_POINTER);
    lv_indev_set_read_cb(indev, my_touchpad_read); // set the touchpad read function

    setup_ui(scr); // pass the active screen object to the sliders setup function

    // set up the RGB-LED
    pinMode(CYD_LED_RED, OUTPUT);
    pinMode(CYD_LED_GREEN, OUTPUT);
    pinMode(CYD_LED_BLUE, OUTPUT);

    Serial.println("Setup done");
}

void loop() {
    lv_tick_inc(millis() - lastTick); // update the tick timer
    lastTick = millis();
    lv_timer_handler(); // update the LVGL UI

    // set the RGB-LED colors using the global sliderColor variable
    // the value if inverted - 0 makes the light glow maximally and 255 makes the light the darkest possible
    // a filter is applied to balance the colors
    analogWrite(CYD_LED_RED, (255 - (sliderColor.red * RED_FILTER)));
    analogWrite(CYD_LED_GREEN, (255 - (sliderColor.green * GREEN_FILTER)));
    analogWrite(CYD_LED_BLUE, (255 - (sliderColor.blue * BLUE_FILTER)));

    delay(5);
}
