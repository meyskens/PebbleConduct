#include "ui_manager.h"
#include "data.h"

// UI Layers - Lamp page
Layer *s_lamp_page_container = NULL;
Layer *s_lamp_bg_layer = NULL;
bool s_lamp_active = false;

// UI Layers - Main page
TextLayer *s_time_layer = NULL;
TextLayer *s_train_number_layer = NULL;
TextLayer *s_station_layer = NULL;
TextLayer *s_status_layer = NULL;
TextLayer *s_next_station_layer = NULL;
TextLayer *s_next_commercial_stop_layer = NULL;
Layer *s_train_info_bg_layer = NULL;
Layer *s_next_station_bg_layer = NULL;
BitmapLayer *s_train_icon_layer = NULL;
GBitmap *s_train_icon_bitmap = NULL;
TextLayer *s_loading_layer = NULL;

// UI Layers - Timetable page
TextLayer *s_timetable_title_layer = NULL;
TextLayer *s_timetable_stop_layers[MAX_STOPS] = {NULL};
TextLayer *s_timetable_time_layers[MAX_STOPS] = {NULL};  // Time in HH:MM
TextLayer *s_timetable_delay_layers[MAX_STOPS] = {NULL}; // Delay (+x/-x)
Layer *s_timetable_row_bg_layers[MAX_STOPS] = {NULL}; // Zebra striping backgrounds
ScrollLayer *s_timetable_scroll_layer = NULL;
Layer *s_timetable_content_layer = NULL;
BitmapLayer *s_timetable_icon_layers[MAX_STOPS] = {NULL}; // Icon layers for stop type
GBitmap *s_train_signal_bitmap = NULL;
GBitmap *s_train_station_bitmap = NULL;

// Status bar
StatusBarLayer *s_status_bar = NULL;

// Animation container layers
static Layer *s_main_page_container = NULL;
static Layer *s_timetable_page_container = NULL;

void train_info_bg_update_proc(Layer *layer, GContext *ctx) {
  graphics_context_set_fill_color(ctx, GColorCeleste);
  graphics_fill_rect(ctx, layer_get_bounds(layer), 0, GCornerNone);
}

void next_station_bg_update_proc(Layer *layer, GContext *ctx) {
  graphics_context_set_fill_color(ctx, GColorPictonBlue);
  graphics_fill_rect(ctx, layer_get_bounds(layer), 0, GCornerNone);
}

// Background color update proc for zebra striping - draws all rows at once
void timetable_row_bg_update_proc(Layer *layer, GContext *ctx) {
  GRect bounds = layer_get_bounds(layer);
  int row_height = 26;
  int num_rows = bounds.size.h / row_height;

  for (int i = 0; i < num_rows; i++) {
    GRect row_rect = GRect(0, i * row_height, bounds.size.w, row_height);
    if (i % 2 == 0) {
      graphics_context_set_fill_color(ctx, GColorWhite);
    } else {
      graphics_context_set_fill_color(ctx, GColorCeleste);
    }
    graphics_fill_rect(ctx, row_rect, 0, GCornerNone);
  }
}

void update_page_visibility(void) {
  if (!s_main_page_container || !s_timetable_page_container) {
    return;
  }
  
  bool main_visible = (s_current_page == PAGE_MAIN);
  bool timetable_visible = (s_current_page == PAGE_TIMETABLE);
  
  // Show/hide entire page containers
  layer_set_hidden(s_main_page_container, !main_visible);
  layer_set_hidden(s_timetable_page_container, !timetable_visible);
  
  // Lamp page visibility is controlled by lamp animation, not page state
  
  // Get screen width from container bounds
  GRect bounds = layer_get_bounds(s_main_page_container);
  
  // Reset positions when not animating
  if (main_visible) {
    GRect frame = layer_get_frame(s_main_page_container);
    frame.origin.y = 0;
    layer_set_frame(s_main_page_container, frame);
    
    GRect tt_frame = layer_get_frame(s_timetable_page_container);
    tt_frame.origin.y = bounds.size.h;
    layer_set_frame(s_timetable_page_container, tt_frame);
  } else {
    GRect frame = layer_get_frame(s_main_page_container);
    frame.origin.y = -bounds.size.h;
    layer_set_frame(s_main_page_container, frame);
    
    GRect tt_frame = layer_get_frame(s_timetable_page_container);
    tt_frame.origin.y = 0;
    layer_set_frame(s_timetable_page_container, tt_frame);
  }
}
