#include "timetable.h"
#include "data.h"
#include "train_info.h"
#include "ui_manager.h"

// Static buffers for text (need persistent storage for text_layer)
static char s_name_buffers[MAX_STOPS][64];
static char s_time_buffers[MAX_STOPS][16];
static char s_delay_buffers[MAX_STOPS][16];

void update_timetable(void) {
  // Use filtered full timetable data (2 previous + current + 5 next)
  // The JS already filters the data, so we just display what we have
  int display_count = s_full_timetable_count;
  if (display_count > MAX_STOPS) {
    display_count = MAX_STOPS;
  }

  // Find current position in filtered data
  int current_in_filtered = -1;
  for (int i = 0; i < s_full_timetable_count; i++) {
    // Check if this entry corresponds to current station
    if (strcmp(s_full_timetable[i].name, s_current_station) == 0) {
      current_in_filtered = i;
      break;
    }
  }

  for (int i = 0; i < display_count && i < MAX_STOPS; i++) {
    TimetableEntry *entry = &s_full_timetable[i];

    // Determine if this station is in the past (before current)
    bool is_past = (i < current_in_filtered);
    bool is_current = (i == current_in_filtered);

    // Update stop name
    if (is_current) {
      snprintf(s_name_buffers[i], sizeof(s_name_buffers[i]), "> %s", entry->name);
    } else {
      strncpy(s_name_buffers[i], entry->name, sizeof(s_name_buffers[i]) - 1);
      s_name_buffers[i][sizeof(s_name_buffers[i]) - 1] = '\0';
    }
    text_layer_set_text(s_timetable_stop_layers[i], s_name_buffers[i]);

    // Set name color: grey for past, black for current/future
    if (is_past) {
      text_layer_set_text_color(s_timetable_stop_layers[i], GColorDarkGray);
    } else {
      text_layer_set_text_color(s_timetable_stop_layers[i], GColorBlack);
    }

    // Update time - show "arrival -> departure" if both available, otherwise just one
    if (entry->arrival[0] && entry->departure[0]) {
      int len = snprintf(s_time_buffers[i], sizeof(s_time_buffers[i]), "%s -> %s", entry->arrival, entry->departure);
      // If too long, truncate to "XX:XX-..."
      if (len >= (int)sizeof(s_time_buffers[i])) {
        snprintf(s_time_buffers[i], sizeof(s_time_buffers[i]), "%s-...", entry->arrival);
      }
    } else if (entry->departure[0]) {
      strncpy(s_time_buffers[i], entry->departure, sizeof(s_time_buffers[i]) - 1);
      s_time_buffers[i][sizeof(s_time_buffers[i]) - 1] = '\0';
    } else if (entry->arrival[0]) {
      strncpy(s_time_buffers[i], entry->arrival, sizeof(s_time_buffers[i]) - 1);
      s_time_buffers[i][sizeof(s_time_buffers[i]) - 1] = '\0';
    } else {
      strcpy(s_time_buffers[i], "--:--");
    }
    text_layer_set_text(s_timetable_time_layers[i], s_time_buffers[i]);

    // Set time color: grey for past, black for current/future (for readability)
    if (is_past) {
      text_layer_set_text_color(s_timetable_time_layers[i], GColorDarkGray);
    } else {
      text_layer_set_text_color(s_timetable_time_layers[i], GColorBlack);
    }

    // Calculate delay - use departure delay if available, otherwise arrival
    // DELAY_NO_INFO (-999) means no delay information from API
    int delay = entry->departure_delay;
    if ((delay == 0 || delay == DELAY_NO_INFO) && entry->arrival_delay != 0 && entry->arrival_delay != DELAY_NO_INFO) {
      delay = entry->arrival_delay;
    }

    // Format delay string
    format_time_offset(s_delay_buffers[i], sizeof(s_delay_buffers[i]), delay);
    text_layer_set_text(s_timetable_delay_layers[i], s_delay_buffers[i]);

    // Set delay color: grey for past, red if delay > +2, otherwise green/black
    if (is_past) {
      text_layer_set_text_color(s_timetable_delay_layers[i], GColorDarkGray);
    } else {
      // Red for delay > +2, otherwise use the standard delay color (green for on-time)
      if (delay > 2) {
        text_layer_set_text_color(s_timetable_delay_layers[i], GColorRed);
      } else {
        text_layer_set_text_color(s_timetable_delay_layers[i], GColorIslamicGreen);
      }
    }

    // Set icon based on commercial_stop flag
    if (entry->commercial_stop) {
      bitmap_layer_set_bitmap(s_timetable_icon_layers[i], s_train_station_bitmap);
    } else {
      bitmap_layer_set_bitmap(s_timetable_icon_layers[i], s_train_signal_bitmap);
    }
  }

  // Clear any unused rows
  for (int i = display_count; i < MAX_STOPS; i++) {
    text_layer_set_text(s_timetable_stop_layers[i], "");
    text_layer_set_text(s_timetable_time_layers[i], "");
    text_layer_set_text(s_timetable_delay_layers[i], "");
    bitmap_layer_set_bitmap(s_timetable_icon_layers[i], NULL);
  }

  // Resize scroll content to actual data size (prevents scrolling empty space)
  if (s_timetable_scroll_layer != NULL && s_timetable_content_layer != NULL) {
    int row_height = 26;
    int actual_height = display_count * row_height;
    GRect current_bounds = layer_get_bounds(s_timetable_content_layer);
    // Only resize if different (avoid unnecessary redraws)
    if (current_bounds.size.h != actual_height) {
      layer_set_bounds(s_timetable_content_layer, GRect(0, 0, current_bounds.size.w, actual_height));
      scroll_layer_set_content_size(s_timetable_scroll_layer, GSize(current_bounds.size.w, actual_height));
    }
  }

  // Auto-scroll to show current station (if found)
  if (current_in_filtered >= 0 && s_timetable_scroll_layer != NULL) {
    int row_height = 26;
    // Scroll so current station is near the top (3rd row)
    int scroll_y = current_in_filtered * row_height - (3 * row_height);
    if (scroll_y < 0) scroll_y = 0;
    scroll_layer_set_content_offset(s_timetable_scroll_layer, GPoint(0, -scroll_y), true);
  }
}
