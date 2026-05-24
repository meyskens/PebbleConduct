#include <pebble.h>

#include "constants.h"
#include "data.h"
#include "window.h"
#include "input_handler.h"
#include "time_display.h"
#include "messaging.h"

// App initialization
static void init() {
  // Initialize data
  init_data();
  
  // Initialize messaging system
  messaging_init();
  
  // Create main window
  s_main_window = window_create();
  
  // Set click config provider
  window_set_click_config_provider(s_main_window, click_config_provider);
  
  // Set window handlers
  window_set_window_handlers(s_main_window, (WindowHandlers) {
    .load = main_window_load,
    .unload = main_window_unload
  });
  
  // Push window to stack
  window_stack_push(s_main_window, true);
  
  // Subscribe to tick timer service (second updates for HH:MM:SS display)
  tick_timer_service_subscribe(SECOND_UNIT, tick_handler);
  
  // Register AppMessage callbacks
  app_message_register_inbox_received(inbox_received_callback);
  app_message_register_inbox_dropped(inbox_dropped_callback);
  app_message_register_outbox_failed(outbox_failed_callback);
  app_message_register_outbox_sent(outbox_sent_callback);
  
  // Open AppMessage with reasonable buffer sizes
  app_message_open(app_message_inbox_size_maximum(), app_message_outbox_size_maximum());
  APP_LOG(APP_LOG_LEVEL_INFO, "AppMessage opened - waiting for JS ready signal");
}

// App deinitialization
static void deinit() {
  // Unsubscribe from services
  tick_timer_service_unsubscribe();
  
  // Cancel any pending timers
  messaging_deinit();
  
  // Destroy window
  window_destroy(s_main_window);
}

// Main entry point
int main(void) {
  init();
  app_event_loop();
  deinit();
}
