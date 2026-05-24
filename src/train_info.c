#include "train_info.h"
#include "data.h"
#include "ui_manager.h"
#include <pebble.h>

void format_time_offset(char *buffer, size_t size, int delay_minutes) {
  if (delay_minutes == DELAY_NO_INFO) {
    // No delay information available - show empty string
    buffer[0] = '\0';
  } else if (delay_minutes > 0) {
    snprintf(buffer, size, "+%d", delay_minutes);
  } else if (delay_minutes < 0) {
    snprintf(buffer, size, "%d", delay_minutes);
  } else {
    snprintf(buffer, size, "+0");
  }
}

GColor get_delay_color(int delay_minutes) {
  if (delay_minutes <= 1) {
    return GColorIslamicGreen;
  } else {
    return GColorRed;
  }
}

void update_train_info(void) {
  // Commercial train number
  text_layer_set_text(s_train_number_layer, s_commercial_train_number);

  // Current station
  snprintf(s_station_buffer, sizeof(s_station_buffer), "%s", s_current_station);
  text_layer_set_text(s_station_layer, s_station_buffer);

  // Status: shows arrival/departure times with delays
  char arrival_delay_str[16];
  char departure_delay_str[16];
  format_time_offset(arrival_delay_str, sizeof(arrival_delay_str), s_arrival_delay);
  format_time_offset(departure_delay_str, sizeof(departure_delay_str), s_departure_delay);

  bool has_arrival = (strlen(s_arrival_time) > 0);
  bool has_departure = (strlen(s_departure_time) > 0);
  bool has_arrival_delay = (s_arrival_delay != DELAY_NO_INFO);
  bool has_departure_delay = (s_departure_delay != DELAY_NO_INFO);

  if (has_arrival && has_departure) {
    // Show both arrival and departure times
    if (has_arrival_delay && has_departure_delay) {
      snprintf(s_status_buffer, sizeof(s_status_buffer), "Arr: %s (%s)\nDep: %s (%s)",
               s_arrival_time, arrival_delay_str, s_departure_time, departure_delay_str);
    } else if (has_arrival_delay) {
      snprintf(s_status_buffer, sizeof(s_status_buffer), "Arr: %s (%s)\nDep: %s",
               s_arrival_time, arrival_delay_str, s_departure_time);
    } else if (has_departure_delay) {
      snprintf(s_status_buffer, sizeof(s_status_buffer), "Arr: %s\nDep: %s (%s)",
               s_arrival_time, s_departure_time, departure_delay_str);
    } else {
      snprintf(s_status_buffer, sizeof(s_status_buffer), "Arr: %s\nDep: %s",
               s_arrival_time, s_departure_time);
    }
  } else if (has_departure) {
    // Only departure time
    if (has_departure_delay) {
      snprintf(s_status_buffer, sizeof(s_status_buffer), "Dep: %s (%s)", s_departure_time, departure_delay_str);
    } else {
      snprintf(s_status_buffer, sizeof(s_status_buffer), "Dep: %s", s_departure_time);
    }
  } else if (has_arrival) {
    // Only arrival time
    if (has_arrival_delay) {
      snprintf(s_status_buffer, sizeof(s_status_buffer), "Arr: %s (%s)", s_arrival_time, arrival_delay_str);
    } else {
      snprintf(s_status_buffer, sizeof(s_status_buffer), "Arr: %s", s_arrival_time);
    }
  } else {
    // Fallback if no time available
    if (s_delay_minutes > 0) {
      snprintf(s_status_buffer, sizeof(s_status_buffer), "Delayed %d min", s_delay_minutes);
    } else {
      snprintf(s_status_buffer, sizeof(s_status_buffer), "On Time");
    }
  }
  text_layer_set_text(s_status_layer, s_status_buffer);
  
  // Position status layer
  GRect status_frame = layer_get_frame(text_layer_get_layer(s_status_layer));
  status_frame.origin.y = 74;
  layer_set_frame(text_layer_get_layer(s_status_layer), status_frame);
  
  // Calculate Y position for next station based on status content
  int y_offset = 78 + ((has_arrival && has_departure) ? 36 : 24);  // More space if showing both times

  // Position next station background layer
  GRect bg_frame = layer_get_frame(s_next_station_bg_layer);
  bg_frame.origin.y = y_offset - 4;  // Background starts just above the station name
  layer_set_frame(s_next_station_bg_layer, bg_frame);

  // Position next station layer
  GRect next_frame = layer_get_frame(text_layer_get_layer(s_next_station_layer));
  next_frame.origin.y = y_offset;
  layer_set_frame(text_layer_get_layer(s_next_station_layer), next_frame);

  // Next station - just the station name
  bool has_next_station = strlen(s_next_station_name) > 0;

  if (has_next_station) {
    snprintf(s_next_station_buffer, sizeof(s_next_station_buffer), "%s", s_next_station_name);
  } else {
    snprintf(s_next_station_buffer, sizeof(s_next_station_buffer), "--");
  }
  text_layer_set_text(s_next_station_layer, s_next_station_buffer);

  // Position next commercial stop layer (now shows arrival/departure times for next station)
  GRect commercial_frame = layer_get_frame(text_layer_get_layer(s_next_commercial_stop_layer));
  commercial_frame.origin.y = y_offset + 16;  // Below the station name
  layer_set_frame(text_layer_get_layer(s_next_commercial_stop_layer), commercial_frame);

  // Show arrival/departure times for next station with labels
  static char next_times_buffer[64];
  bool has_next_arrival = strlen(s_next_station_arrival) > 0;
  bool has_next_departure = strlen(s_next_station_departure) > 0;

  if (has_next_arrival && has_next_departure) {
    snprintf(next_times_buffer, sizeof(next_times_buffer), "Arr: %s  Dep: %s",
             s_next_station_arrival, s_next_station_departure);
  } else if (has_next_departure) {
    snprintf(next_times_buffer, sizeof(next_times_buffer), "Dep: %s", s_next_station_departure);
  } else if (has_next_arrival) {
    snprintf(next_times_buffer, sizeof(next_times_buffer), "Arr: %s", s_next_station_arrival);
  } else {
    next_times_buffer[0] = '\0';
  }
  text_layer_set_text(s_next_commercial_stop_layer, next_times_buffer);
}
