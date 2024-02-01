// This file was generated by SquareLine Studio
// SquareLine Studio version: SquareLine Studio 1.3.4
// LVGL version: 8.3.6
// Project name: SquareLine_Project

#include "ui.h"

void ui_Screen1_screen_init(void)
{
    ui_Screen1 = lv_obj_create(NULL);
    lv_obj_clear_flag(ui_Screen1, LV_OBJ_FLAG_SCROLLABLE);      /// Flags

    ui_Panel1 = lv_obj_create(ui_Screen1);
    lv_obj_set_width(ui_Panel1, lv_pct(90));
    lv_obj_set_height(ui_Panel1, lv_pct(90));
    lv_obj_set_align(ui_Panel1, LV_ALIGN_CENTER);
    lv_obj_clear_flag(ui_Panel1, LV_OBJ_FLAG_SCROLLABLE);      /// Flags

    ui_Label1 = lv_label_create(ui_Panel1);
    lv_obj_set_width(ui_Label1, lv_pct(100));
    lv_obj_set_height(ui_Label1, lv_pct(100));
    lv_obj_set_align(ui_Label1, LV_ALIGN_CENTER);
    lv_label_set_text(ui_Label1, "UI  Loaded!\n\nDrop your UI in the UI Directory\n\n");

    ui_Slider1 = lv_slider_create(ui_Screen1);
    lv_obj_set_width(ui_Slider1, 242);
    lv_obj_set_height(ui_Slider1, 10);
    lv_obj_set_x(ui_Slider1, 4);
    lv_obj_set_y(ui_Slider1, 80);
    lv_obj_set_align(ui_Slider1, LV_ALIGN_CENTER);


    ui_Label2 = lv_label_create(ui_Screen1);
    lv_obj_set_width(ui_Label2, LV_SIZE_CONTENT);   /// 1
    lv_obj_set_height(ui_Label2, LV_SIZE_CONTENT);    /// 1
    lv_obj_set_x(ui_Label2, -8);
    lv_obj_set_y(ui_Label2, 44);
    lv_obj_set_align(ui_Label2, LV_ALIGN_CENTER);
    lv_label_set_text(ui_Label2, "Touch Test");
    lv_label_set_recolor(ui_Label2, "true");

}