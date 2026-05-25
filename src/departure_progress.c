#include "departure_progress.h"
#include "data.h"
#include <pebble.h>

// Progress bar layer
Layer *s_departure_progress_layer = NULL;

// Progress value 0-100 (0 = full, 100 = empty, counting down to departure)
static int s_progress_percent = 0;
static bool s_vibration_triggered = false;
static bool s_in_warning_phase = false;

// Countdown duration in seconds (30 seconds before departure)
#define COUNTDOWN_DURATION_SEC 30

void departure_progress_draw_proc(Layer *layer, GContext *ctx) {
  GRect bounds = layer_get_bounds(layer);
  
  // Background bar (light gray)
  graphics_context_set_fill_color(ctx, GColorLightGray);
  graphics_fill_rect(ctx, bounds, 2, GCornersAll);
  
  if (s_in_warning_phase) {
    // Departure phase: empty bar pulsing red
    time_t now = time(NULL);
    bool pulse_on = (now % 2) == 0; // Toggle every second
    
    if (pulse_on) {
      // Draw red outline/empty bar
      graphics_context_set_fill_color(ctx, GColorRed);
      GRect red_rect = GRect(bounds.origin.x, bounds.origin.y, bounds.size.w, bounds.size.h);
      graphics_fill_rect(ctx, red_rect, 2, GCornersAll);
    }
  } else {
    // Normal progress fill (black) - decreases as we approach departure
    if (s_progress_percent > 0) {
      int fill_width = (bounds.size.w * s_progress_percent) / 100;
      GRect fill_rect = GRect(bounds.origin.x, bounds.origin.y, fill_width, bounds.size.h);
      graphics_context_set_fill_color(ctx, GColorBlack);
      graphics_fill_rect(ctx, fill_rect, 2, GCornersAll);
    }
  }
}

void departure_progress_init(void) {
  // Create layer with fixed size - will be positioned by window.c
  s_departure_progress_layer = layer_create(GRect(0, 0, 144, 6));
  layer_set_update_proc(s_departure_progress_layer, departure_progress_draw_proc);
  layer_set_hidden(s_departure_progress_layer, true);
  s_vibration_triggered = false;
}

void departure_progress_destroy(void) {
  if (s_departure_progress_layer) {
    layer_destroy(s_departure_progress_layer);
    s_departure_progress_layer = NULL;
  }
}

bool departure_progress_should_show(void) {
  // Show if current stop has both arrival and departure times
  // The progress bar shows dwell time from arrival to (departure - 30s)
  bool has_arrival = (strlen(s_arrival_time) > 0);
  bool has_departure = (strlen(s_departure_time) > 0);
  return has_arrival && has_departure;
}

// Parse HH:MM time string to minutes since midnight
static int parse_time_to_minutes(const char *time_str) {
  if (!time_str || strlen(time_str) < 5) return -1;
  
  int hours = (time_str[0] - '0') * 10 + (time_str[1] - '0');
  int minutes = (time_str[3] - '0') * 10 + (time_str[4] - '0');
  return hours * 60 + minutes;
}

bool departure_progress_update(void) {
  if (!s_departure_progress_layer) return false;

  // Check if we should show the progress bar
  if (!departure_progress_should_show()) {
    layer_set_hidden(s_departure_progress_layer, true);
    s_vibration_triggered = false;
    return false;
  }

  // Get current time
  time_t now = time(NULL);
  struct tm *now_tm = localtime(&now);
  int current_seconds = now_tm->tm_hour * 3600 + now_tm->tm_min * 60 + now_tm->tm_sec;

  // Parse arrival and departure times
  int arr_minutes = parse_time_to_minutes(s_arrival_time);
  int dep_minutes = parse_time_to_minutes(s_departure_time);
  if (arr_minutes < 0 || dep_minutes < 0) {
    layer_set_hidden(s_departure_progress_layer, true);
    return false;
  }

  // Convert to seconds
  int arr_seconds = arr_minutes * 60;
  int dep_seconds = dep_minutes * 60;

  // Calculate countdown window: from arrival to (departure - 30 seconds)
  int countdown_end = dep_seconds - COUNTDOWN_DURATION_SEC;
  int total_countdown_duration = countdown_end - arr_seconds;

  // Don't show before arrival
  if (current_seconds < arr_seconds) {
    layer_set_hidden(s_departure_progress_layer, true);
    s_vibration_triggered = false;
    s_in_warning_phase = false;
    return false;
  }

  // Show the progress bar (visible from arrival until departure)
  layer_set_hidden(s_departure_progress_layer, false);

  // Check if we're in the departure phase (countdown complete, but not yet departed)
  if (current_seconds >= countdown_end) {
    s_in_warning_phase = true;
    s_progress_percent = 0;
    
    // Trigger vibration when entering warning phase
    if (!s_vibration_triggered) {
      // Vibe pattern: ON for 200ms, OFF for 100ms, ON for 400ms:
      static const uint32_t segments[] = { 200, 100, 500 };
      VibePattern pat = {
        .durations = segments,
        .num_segments = ARRAY_LENGTH(segments),
      };
      vibes_enqueue_custom_pattern(pat);
      s_vibration_triggered = true;
    }
    
    // Mark layer dirty to trigger redraw (for pulsing effect)
    layer_mark_dirty(s_departure_progress_layer);
    return true;
  }

  // Normal countdown phase
  s_in_warning_phase = false;
  s_vibration_triggered = false;

  // Calculate progress (100% = full bar at arrival, 0% = empty at countdown end)
  int elapsed = current_seconds - arr_seconds;
  s_progress_percent = 100 - ((elapsed * 100) / total_countdown_duration);
  if (s_progress_percent < 0) s_progress_percent = 0;
  if (s_progress_percent > 100) s_progress_percent = 100;

  // Mark layer dirty to trigger redraw
  layer_mark_dirty(s_departure_progress_layer);

  return true;
}

Layer* departure_progress_get_layer(void) {
  return s_departure_progress_layer;
}
