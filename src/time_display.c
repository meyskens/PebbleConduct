#include "time_display.h"
#include "data.h"
#include "ui_manager.h"

void update_time(void) {
  time_t temp = time(NULL);
  struct tm *tick_time = localtime(&temp);

  // Format time with seconds (always 24h)
  strftime(s_time_buffer, sizeof(s_time_buffer), "%H:%M:%S", tick_time);

  // Update display
  text_layer_set_text(s_time_layer, s_time_buffer);
}

void tick_handler(struct tm *tick_time, TimeUnits units_changed) {
  update_time();
}
