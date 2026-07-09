//----------------------------------------------------------------------
// LVGL_UI.cpp
// ===========
// Edit the CreateExampleScreen() function to select what you want to
// display on the screen.
//----------------------------------------------------------------------
#include <lvgl.h>

//----------------------------------------------------------------------
// Draw some labels
// Example from the LVGL documentation
// https://docs.lvgl.io/master/widgets/label.html
//----------------------------------------------------------------------
void draw_some_labels(void)
{
  lv_obj_t * label1 = lv_label_create(lv_screen_active());
  lv_label_set_long_mode(label1, LV_LABEL_LONG_MODE_WRAP);     /*Break the long lines*/
  lv_label_set_recolor(label1, true);                      /*Enable re-coloring by commands in the text*/
  lv_label_set_text(label1, "#0000ff Re-color# #ff00ff words# #ff0000 of a# label, align the lines to the center and wrap long text automatically.");
  lv_obj_set_width(label1, 150);  /*Set smaller width to make the lines wrap*/
  lv_obj_set_style_text_align(label1, LV_TEXT_ALIGN_CENTER, 0);
  lv_obj_align(label1, LV_ALIGN_CENTER, 0, -40);

  lv_obj_t * label2 = lv_label_create(lv_screen_active());
  lv_label_set_long_mode(label2, LV_LABEL_LONG_MODE_SCROLL_CIRCULAR);     /*Circular scroll*/
  lv_obj_set_width(label2, 150);
  lv_label_set_text(label2, "It is a circularly scrolling text. ");
  lv_obj_align(label2, LV_ALIGN_CENTER, 0, 40);
}

//----------------------------------------------------------------------
// Draw some controls
// Example from Random Nerd Tutorials site
// https://randomnerdtutorials.com/lvgl-cheap-yellow-display-esp32-2432s028r/
//----------------------------------------------------------------------
int btn1_count = 0;
// Callback that is triggered when btn1 is clicked
static void event_handler_btn1(lv_event_t * e) {
  lv_event_code_t code = lv_event_get_code(e);
  if(code == LV_EVENT_CLICKED) {
    btn1_count++;
    LV_LOG_USER("Button clicked %d", (int)btn1_count);
  }
}

// Callback that is triggered when btn2 is clicked/toggled
static void event_handler_btn2(lv_event_t * e) {
  lv_event_code_t code = lv_event_get_code(e);
  lv_obj_t * obj = (lv_obj_t*) lv_event_get_target(e);
  if(code == LV_EVENT_VALUE_CHANGED) {
    LV_UNUSED(obj);
    LV_LOG_USER("Toggled %s", lv_obj_has_state(obj, LV_STATE_CHECKED) ? "on" : "off");
  }
}

static lv_obj_t * slider_label;
// Callback that prints the current slider value on the TFT display and Serial Monitor for debugging purposes
static void slider_event_callback(lv_event_t * e) {
  lv_obj_t * slider = (lv_obj_t*) lv_event_get_target(e);
  char buf[8];
  lv_snprintf(buf, sizeof(buf), "%d%%", (int)lv_slider_get_value(slider));
  lv_label_set_text(slider_label, buf);
  lv_obj_align_to(slider_label, slider, LV_ALIGN_OUT_BOTTOM_MID, 0, 10);
  LV_LOG_USER("Slider changed to %d%%", (int)lv_slider_get_value(slider));
}

void draw_some_controls(void) {
  // Create a text label aligned center on top ("Hello, world!")
  lv_obj_t * text_label = lv_label_create(lv_screen_active());
  lv_label_set_long_mode(text_label, LV_LABEL_LONG_WRAP);    // Breaks the long lines
  lv_label_set_text(text_label, "Hello, world!");
  lv_obj_set_width(text_label, 150);    // Set smaller width to make the lines wrap
  lv_obj_set_style_text_align(text_label, LV_TEXT_ALIGN_CENTER, 0);
  lv_obj_align(text_label, LV_ALIGN_CENTER, 0, -90);

  lv_obj_t * btn_label;
  // Create a Button (btn1)
  lv_obj_t * btn1 = lv_button_create(lv_screen_active());
  lv_obj_add_event_cb(btn1, event_handler_btn1, LV_EVENT_ALL, NULL);
  lv_obj_align(btn1, LV_ALIGN_CENTER, 0, -50);
  lv_obj_remove_flag(btn1, LV_OBJ_FLAG_PRESS_LOCK);

  btn_label = lv_label_create(btn1);
  lv_label_set_text(btn_label, "Button");
  lv_obj_center(btn_label);

  // Create a Toggle button (btn2)
  lv_obj_t * btn2 = lv_button_create(lv_screen_active());
  lv_obj_add_event_cb(btn2, event_handler_btn2, LV_EVENT_ALL, NULL);
  lv_obj_align(btn2, LV_ALIGN_CENTER, 0, 10);
  lv_obj_add_flag(btn2, LV_OBJ_FLAG_CHECKABLE);
  lv_obj_set_height(btn2, LV_SIZE_CONTENT);

  btn_label = lv_label_create(btn2);
  lv_label_set_text(btn_label, "Toggle");
  lv_obj_center(btn_label);
  
  // Create a slider aligned in the center bottom of the TFT display
  lv_obj_t * slider = lv_slider_create(lv_screen_active());
  lv_obj_align(slider, LV_ALIGN_CENTER, 0, 60);
  lv_obj_add_event_cb(slider, slider_event_callback, LV_EVENT_VALUE_CHANGED, NULL);
  lv_slider_set_range(slider, 0, 100);
  lv_obj_set_style_anim_duration(slider, 2000, 0);

  // Create a label below the slider to display the current slider value
  slider_label = lv_label_create(lv_screen_active());
  lv_label_set_text(slider_label, "0%");
  lv_obj_align_to(slider_label, slider, LV_ALIGN_OUT_BOTTOM_MID, 0, 10);
}

//----------------------------------------------------------------------
// Draw a chart
// Example from the LVGL documentation
// https://docs.lvgl.io/master/widgets/chart.html
//----------------------------------------------------------------------
void draw_a_chart(void)
{
  /*Create a chart*/
  lv_obj_t * chart;
  chart = lv_chart_create(lv_screen_active());
  lv_obj_set_size(chart, 200, 150);
  lv_obj_center(chart);
  lv_chart_set_type(chart, LV_CHART_TYPE_LINE);   /*Show lines and points too*/

  /*Add two data series*/
  lv_chart_series_t * ser1 = lv_chart_add_series(chart, lv_palette_main(LV_PALETTE_GREEN), LV_CHART_AXIS_PRIMARY_Y);
  lv_chart_series_t * ser2 = lv_chart_add_series(chart, lv_palette_main(LV_PALETTE_RED), LV_CHART_AXIS_SECONDARY_Y);
  int32_t * ser2_y_points = lv_chart_get_series_y_array(chart, ser2);

  uint32_t i;
  for(i = 0; i < 10; i++) {
    /*Set the next points on 'ser1'*/
    lv_chart_set_next_value(chart, ser1, (int32_t)lv_rand(10, 50));

    /*Directly set points on 'ser2'*/
    ser2_y_points[i] = (int32_t)lv_rand(50, 90);
  }

  lv_obj_set_style_drop_shadow_opa(chart, 255, LV_PART_ITEMS);
  lv_obj_set_style_drop_shadow_offset_x(chart, 0, LV_PART_ITEMS);
  lv_obj_set_style_drop_shadow_radius(chart, 20, LV_PART_ITEMS);
  /* Drop shadow color follows each series color automatically,
   * so no manual override is needed. */
  lv_chart_refresh(chart); /*Required after direct set*/
}

//----------------------------------------------------------------------
// CreateExampleScreen
// -------------------
// Create some widgets on the screen as an example
// Edit this function to choose what you want to draw
//----------------------------------------------------------------------
void CreateExampleScreen() {
  // draw_some_labels();
  draw_some_controls();
  // draw_a_chart();
}