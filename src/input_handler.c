#include "input_handler.h"
#include "data.h"
#include "train_info.h"
#include "ui_manager.h"
#include "animations.h"

// Constants for scroll behavior
#define SCROLL_ROW_HEIGHT 26
#define SCROLL_TOP_THRESHOLD 0

// Long click delay for lamp (ms)
#define LAMP_LONG_CLICK_DELAY 500

void select_long_click_handler(ClickRecognizerRef recognizer, void *context) {
  // Long press on middle button - activate lamp
  start_lamp_animation();
}

void back_click_handler(ClickRecognizerRef recognizer, void *context) {
  if (is_lamp_active()) {
    // Back button turns off lamp
    stop_lamp_animation();
    return;
  }
  // Normal back behavior - go to main page or close app if already on main page
  if (s_current_page > PAGE_MAIN && !is_animation_running()) {
    animate_to_page(PAGE_MAIN);
  } else if (s_current_page == PAGE_MAIN) {
    // Already on main page, close the app
    window_stack_pop_all(true);
  }
}

void up_click_handler(ClickRecognizerRef recognizer, void *context) {
  if (is_lamp_active()) {
    // Ignore when lamp is active
    return;
  }
  if (s_current_page == PAGE_TIMETABLE && s_timetable_scroll_layer) {
    // Scroll timetable up
    GPoint offset = scroll_layer_get_content_offset(s_timetable_scroll_layer);
    // If already at the top, go back to main page
    if (offset.y >= SCROLL_TOP_THRESHOLD) {
      if (!is_animation_running()) {
        animate_to_page(PAGE_MAIN);
      }
    } else {
      offset.y += SCROLL_ROW_HEIGHT; // Scroll one row up
      if (offset.y > SCROLL_TOP_THRESHOLD) offset.y = SCROLL_TOP_THRESHOLD;
      scroll_layer_set_content_offset(s_timetable_scroll_layer, offset, true);
    }
  } else if (s_current_page > PAGE_MAIN && !is_animation_running()) {
    animate_to_page(PAGE_MAIN);
  }
}

void down_click_handler(ClickRecognizerRef recognizer, void *context) {
  if (is_lamp_active()) {
    // Ignore when lamp is active
    return;
  }
  if (s_current_page == PAGE_TIMETABLE && s_timetable_scroll_layer) {
    // Scroll timetable down
    GPoint offset = scroll_layer_get_content_offset(s_timetable_scroll_layer);
    offset.y -= SCROLL_ROW_HEIGHT; // Scroll one row down
    // Clamp to bottom (content height - scroll height)
    GSize content_size = scroll_layer_get_content_size(s_timetable_scroll_layer);
    GRect scroll_bounds = layer_get_bounds(scroll_layer_get_layer(s_timetable_scroll_layer));
    int min_y = scroll_bounds.size.h - content_size.h;
    if (offset.y < min_y) offset.y = min_y;
    scroll_layer_set_content_offset(s_timetable_scroll_layer, offset, true);
  } else if (s_current_page < PAGE_TIMETABLE && !is_animation_running()) {
    animate_to_page(PAGE_TIMETABLE);
  }
}

void click_config_provider(void *context) {
  // SELECT button - long click for lamp (no release handler needed)
  window_long_click_subscribe(BUTTON_ID_SELECT, LAMP_LONG_CLICK_DELAY, 
                              select_long_click_handler, 
                              NULL);
  // BACK button - turns off lamp or goes to main page
  window_single_click_subscribe(BUTTON_ID_BACK, back_click_handler);
  window_single_click_subscribe(BUTTON_ID_UP, up_click_handler);
  window_single_click_subscribe(BUTTON_ID_DOWN, down_click_handler);
}
