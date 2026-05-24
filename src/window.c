#include "window.h"
#include "data.h"
#include "ui_manager.h"
#include "time_display.h"
#include "train_info.h"
#include "timetable.h"
#include "animations.h"

// Main window
Window *s_main_window;

// Content container (holds page containers, sits below status bar)
static Layer *s_content_container = NULL;

// Page container layers for animation (extern in window.h)
Layer *s_main_page_container = NULL;
Layer *s_timetable_page_container = NULL;

void main_window_load(Window *window) {
  Layer *window_layer = window_get_root_layer(window);
  GRect bounds = layer_get_bounds(window_layer);
  
  // Set background color (white for clock section)
  window_set_background_color(window, GColorWhite);
  
  // Status bar (not part of animated containers)
  s_status_bar = status_bar_layer_create();
  status_bar_layer_set_colors(s_status_bar, GColorWhite, GColorBlack);
  layer_add_child(window_layer, status_bar_layer_get_layer(s_status_bar));
  
  // Adjust bounds for status bar
  GRect content_bounds = bounds;
  content_bounds.origin.y += STATUS_BAR_LAYER_HEIGHT;
  content_bounds.size.h -= STATUS_BAR_LAYER_HEIGHT;
  
  // Create content container that sits below status bar
  s_content_container = layer_create(content_bounds);
  layer_add_child(window_layer, s_content_container);
  
  // Get bounds relative to content container
  GRect page_bounds = layer_get_bounds(s_content_container);
  
  // ========== MAIN PAGE CONTAINER ==========
  s_main_page_container = layer_create(page_bounds);
  layer_add_child(s_content_container, s_main_page_container);
  
  // Time layer (top) - compact height
  s_time_layer = text_layer_create(
    GRect(0, 0, bounds.size.w, 28));
  text_layer_set_background_color(s_time_layer, GColorClear);
  text_layer_set_text_color(s_time_layer, GColorBlack);
  text_layer_set_font(s_time_layer, fonts_get_system_font(FONT_KEY_LECO_28_LIGHT_NUMBERS));
  text_layer_set_text_alignment(s_time_layer, GTextAlignmentCenter);
  layer_add_child(s_main_page_container, text_layer_get_layer(s_time_layer));
  
  // Light blue background layer for train info section - starts right after clock
  s_train_info_bg_layer = layer_create(
    GRect(0, 30, bounds.size.w, content_bounds.size.h - 30));
  layer_set_update_proc(s_train_info_bg_layer, train_info_bg_update_proc);
  layer_add_child(s_main_page_container, s_train_info_bg_layer);
  
  // Train icon bitmap layer (in front of train number) - vertically centered with text
  s_train_icon_bitmap = gbitmap_create_with_resource(RESOURCE_ID_TRAIN_ICON);
  s_train_icon_layer = bitmap_layer_create(GRect(5, 40, 18, 18));
  bitmap_layer_set_bitmap(s_train_icon_layer, s_train_icon_bitmap);
  bitmap_layer_set_compositing_mode(s_train_icon_layer, GCompOpSet);
  layer_add_child(s_main_page_container, bitmap_layer_get_layer(s_train_icon_layer));

  // Commercial train number layer - shifted right to make room for icon
  s_train_number_layer = text_layer_create(
    GRect(26, 32, bounds.size.w - 31, 26));
  text_layer_set_background_color(s_train_number_layer, GColorClear);
  text_layer_set_text_color(s_train_number_layer, GColorBlack);
  text_layer_set_font(s_train_number_layer, fonts_get_system_font(FONT_KEY_GOTHIC_24_BOLD));
  text_layer_set_text_alignment(s_train_number_layer, GTextAlignmentLeft);
  layer_add_child(s_main_page_container, text_layer_get_layer(s_train_number_layer));
  
  // Current station layer
  s_station_layer = text_layer_create(
    GRect(5, 56, bounds.size.w - 10, 24));
  text_layer_set_background_color(s_station_layer, GColorClear);
  text_layer_set_text_color(s_station_layer, GColorBlack);
  text_layer_set_font(s_station_layer, fonts_get_system_font(FONT_KEY_GOTHIC_18_BOLD));
  text_layer_set_text_alignment(s_station_layer, GTextAlignmentLeft);
  layer_add_child(s_main_page_container, text_layer_get_layer(s_station_layer));
  
  // Status layer (shows arrival/departure times + delays, can be 2 lines)
  s_status_layer = text_layer_create(
    GRect(5, 78, bounds.size.w - 10, 40));
  text_layer_set_background_color(s_status_layer, GColorClear);
  text_layer_set_text_color(s_status_layer, GColorOxfordBlue);
  text_layer_set_font(s_status_layer, fonts_get_system_font(FONT_KEY_GOTHIC_18_BOLD));
  text_layer_set_text_alignment(s_status_layer, GTextAlignmentLeft);
  layer_add_child(s_main_page_container, text_layer_get_layer(s_status_layer));
  
  // Full-width background for next station section (starts above station name)
  s_next_station_bg_layer = layer_create(
    GRect(0, 104, bounds.size.w, 74));
  layer_set_update_proc(s_next_station_bg_layer, next_station_bg_update_proc);
  layer_add_child(s_main_page_container, s_next_station_bg_layer);

  // Next station layer (single line, black with transparent background)
  s_next_station_layer = text_layer_create(
    GRect(5, 118, bounds.size.w - 10, 18));
  text_layer_set_background_color(s_next_station_layer, GColorClear);
  text_layer_set_text_color(s_next_station_layer, GColorBlack);
  text_layer_set_font(s_next_station_layer, fonts_get_system_font(FONT_KEY_GOTHIC_14));
  text_layer_set_text_alignment(s_next_station_layer, GTextAlignmentLeft);
  layer_add_child(s_main_page_container, text_layer_get_layer(s_next_station_layer));

  // Next commercial stop layer (shows arrival/departure times for next commercial stop)
  s_next_commercial_stop_layer = text_layer_create(
    GRect(5, 136, bounds.size.w - 10, 16));
  text_layer_set_background_color(s_next_commercial_stop_layer, GColorClear);
  text_layer_set_text_color(s_next_commercial_stop_layer, GColorBlack);
  text_layer_set_font(s_next_commercial_stop_layer, fonts_get_system_font(FONT_KEY_GOTHIC_14));
  text_layer_set_text_alignment(s_next_commercial_stop_layer, GTextAlignmentLeft);
  layer_add_child(s_main_page_container, text_layer_get_layer(s_next_commercial_stop_layer));
  
  // ========== TIMETABLE PAGE CONTAINER ==========
  s_timetable_page_container = layer_create(page_bounds);
  // Start off-screen below (relative to content container)
  GRect timetable_frame = page_bounds;
  timetable_frame.origin.y = page_bounds.size.h;
  layer_set_frame(s_timetable_page_container, timetable_frame);
  layer_add_child(s_content_container, s_timetable_page_container);

  // Create scroll layer for timetable (allows scrolling through many stops)
  GRect scroll_bounds = GRect(0, 28, bounds.size.w, content_bounds.size.h - 28); // Below title
  s_timetable_scroll_layer = scroll_layer_create(scroll_bounds);
  // Make scroll layer background transparent so row backgrounds show
  scroll_layer_set_shadow_hidden(s_timetable_scroll_layer, true);
  // Don't set click config onto window - we handle buttons manually for page navigation
  layer_add_child(s_timetable_page_container, scroll_layer_get_layer(s_timetable_scroll_layer));

  // Content layer inside scroll - tall enough for all entries
  int row_height = 26;
  int content_height = MAX_STOPS * row_height; // Just the rows
  s_timetable_content_layer = layer_create(GRect(0, 0, bounds.size.w, content_height));
  scroll_layer_add_child(s_timetable_scroll_layer, s_timetable_content_layer);

  // Timetable title (fixed at top, outside scroll)
  s_timetable_title_layer = text_layer_create(
    GRect(0, 2, bounds.size.w, 24));
  text_layer_set_background_color(s_timetable_title_layer, GColorClear);
  text_layer_set_text_color(s_timetable_title_layer, GColorBlack);
  text_layer_set_font(s_timetable_title_layer, fonts_get_system_font(FONT_KEY_GOTHIC_18_BOLD));
  text_layer_set_text_alignment(s_timetable_title_layer, GTextAlignmentCenter);
  text_layer_set_text(s_timetable_title_layer, "Timetable");
  layer_add_child(s_timetable_page_container, text_layer_get_layer(s_timetable_title_layer));

  // Timetable stop entries - inside scrollable content layer
  int start_y = 0; // Relative to content layer

  // Create a single background layer for zebra striping
  Layer *zebra_bg_layer = layer_create(GRect(0, 0, bounds.size.w, content_height));
  layer_set_update_proc(zebra_bg_layer, timetable_row_bg_update_proc);
  layer_add_child(s_timetable_content_layer, zebra_bg_layer);

  // Load icon bitmaps for stop types
  s_train_signal_bitmap = gbitmap_create_with_resource(RESOURCE_ID_TRAIN_SIGNAL);
  s_train_station_bitmap = gbitmap_create_with_resource(RESOURCE_ID_TRAIN_STATION);

  for (int i = 0; i < MAX_STOPS; i++) {
    // Icon layer for stop type (left of stop name) - full height of row (26px)
    s_timetable_icon_layers[i] = bitmap_layer_create(
      GRect(3, start_y + i * row_height, 20, 26));
    bitmap_layer_set_compositing_mode(s_timetable_icon_layers[i], GCompOpSet);
    layer_add_child(s_timetable_content_layer, bitmap_layer_get_layer(s_timetable_icon_layers[i]));

    // Stop name layer (left side, top line) - shifted right to make room for icon
    s_timetable_stop_layers[i] = text_layer_create(
      GRect(26, start_y + i * row_height, bounds.size.w - 76, 14));
    text_layer_set_background_color(s_timetable_stop_layers[i], GColorClear);
    text_layer_set_text_color(s_timetable_stop_layers[i], GColorBlack);
    text_layer_set_font(s_timetable_stop_layers[i], fonts_get_system_font(FONT_KEY_GOTHIC_14));
    text_layer_set_text_alignment(s_timetable_stop_layers[i], GTextAlignmentLeft);
    layer_add_child(s_timetable_content_layer, text_layer_get_layer(s_timetable_stop_layers[i]));

    // Time layer (left side, bottom line - HH:MM or HH:MM -> HH:MM) - shifted right to align with name
    s_timetable_time_layers[i] = text_layer_create(
      GRect(26, start_y + i * row_height + 12, 100, 14));
    text_layer_set_background_color(s_timetable_time_layers[i], GColorClear);
    text_layer_set_text_color(s_timetable_time_layers[i], GColorDarkGray);
    text_layer_set_font(s_timetable_time_layers[i], fonts_get_system_font(FONT_KEY_GOTHIC_14));
    text_layer_set_text_alignment(s_timetable_time_layers[i], GTextAlignmentLeft);
    layer_add_child(s_timetable_content_layer, text_layer_get_layer(s_timetable_time_layers[i]));

    // Delay layer (right side - bigger text for +x/-x)
    s_timetable_delay_layers[i] = text_layer_create(
      GRect(bounds.size.w - 45, start_y + i * row_height + 6, 40, 18));
    text_layer_set_background_color(s_timetable_delay_layers[i], GColorClear);
    text_layer_set_font(s_timetable_delay_layers[i], fonts_get_system_font(FONT_KEY_GOTHIC_18_BOLD));
    text_layer_set_text_alignment(s_timetable_delay_layers[i], GTextAlignmentRight);
    layer_add_child(s_timetable_content_layer, text_layer_get_layer(s_timetable_delay_layers[i]));
  }

  // Set scroll layer content size
  scroll_layer_set_content_size(s_timetable_scroll_layer, GSize(bounds.size.w, content_height));
  
  // ========== LAMP PAGE CONTAINER ==========
  s_lamp_page_container = layer_create(page_bounds);
  layer_add_child(s_content_container, s_lamp_page_container);
  
  // Lamp background layer (full screen green)
  s_lamp_bg_layer = layer_create(page_bounds);
  layer_add_child(s_lamp_page_container, s_lamp_bg_layer);
  
  // Hide lamp page initially
  layer_set_hidden(s_lamp_page_container, true);
  
  // Loading layer (shown while waiting for data)
  s_loading_layer = text_layer_create(
    GRect(0, content_bounds.size.h / 2 - 20, bounds.size.w, 40));
  text_layer_set_background_color(s_loading_layer, GColorClear);
  text_layer_set_text_color(s_loading_layer, GColorBlack);
  text_layer_set_font(s_loading_layer, fonts_get_system_font(FONT_KEY_GOTHIC_18_BOLD));
  text_layer_set_text_alignment(s_loading_layer, GTextAlignmentCenter);
  text_layer_set_text(s_loading_layer, "Loading...");
  layer_add_child(s_content_container, text_layer_get_layer(s_loading_layer));

  // Initialize animation system first
  init_animations();

  // Set up animation containers
  set_animation_containers(s_main_page_container, s_timetable_page_container, s_lamp_page_container);

  // Initial data display
  update_time();
  update_train_info();
  update_timetable();
  update_page_visibility();

  // Show loading screen initially - hide main page until data arrives
  layer_set_hidden(s_main_page_container, true);
  layer_set_hidden(s_timetable_page_container, true);
}

void main_window_unload(Window *window) {
  // Destroy loading layer (not part of containers)
  text_layer_destroy(s_loading_layer);
  s_loading_layer = NULL;

  // Destroy content container (this also destroys page containers and their children)
  layer_destroy(s_content_container);

  // Reset container pointers
  s_content_container = NULL;
  s_main_page_container = NULL;
  s_timetable_page_container = NULL;

  // Destroy scroll layer (not part of content container)
  scroll_layer_destroy(s_timetable_scroll_layer);
  s_timetable_scroll_layer = NULL;
  s_timetable_content_layer = NULL;

  // Reset layer pointers (already destroyed with containers)
  s_time_layer = NULL;
  s_train_number_layer = NULL;
  s_station_layer = NULL;
  s_status_layer = NULL;
  s_next_station_layer = NULL;
  s_next_commercial_stop_layer = NULL;
  s_train_info_bg_layer = NULL;
  s_next_station_bg_layer = NULL;
  s_timetable_title_layer = NULL;
  
  // Destroy train icon bitmap and layer
  if (s_train_icon_bitmap) {
    gbitmap_destroy(s_train_icon_bitmap);
    s_train_icon_bitmap = NULL;
  }
  s_train_icon_layer = NULL;

  // Destroy stop type icon bitmaps
  if (s_train_signal_bitmap) {
    gbitmap_destroy(s_train_signal_bitmap);
    s_train_signal_bitmap = NULL;
  }
  if (s_train_station_bitmap) {
    gbitmap_destroy(s_train_station_bitmap);
    s_train_station_bitmap = NULL;
  }

  for (int i = 0; i < MAX_STOPS; i++) {
    s_timetable_stop_layers[i] = NULL;
    s_timetable_time_layers[i] = NULL;
    s_timetable_icon_layers[i] = NULL;
    s_timetable_delay_layers[i] = NULL;
  }

  status_bar_layer_destroy(s_status_bar);
  s_status_bar = NULL;
}
