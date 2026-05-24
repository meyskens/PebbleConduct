#include "messaging.h"
#include "data.h"
#include "train_info.h"
#include "ui_manager.h"
#include "timetable.h"
#include "window.h"

// JS Ready state
static bool s_js_ready = false;

// Timer for periodic updates (every minute)
static AppTimer *s_update_timer = NULL;

// Retry timer for failed requests
static AppTimer *s_retry_timer = NULL;
static int s_retry_count = 0;
#define MAX_RETRIES 3
#define RETRY_INTERVAL_MS 5000  // 5 seconds

// Forward declarations
static void schedule_next_update(void);
static void retry_timer_callback(void *data);

void messaging_init(void) {
  s_js_ready = false;
  s_retry_count = 0;
  s_update_timer = NULL;
  s_retry_timer = NULL;
}

void messaging_deinit(void) {
  if (s_update_timer) {
    app_timer_cancel(s_update_timer);
    s_update_timer = NULL;
  }
  if (s_retry_timer) {
    app_timer_cancel(s_retry_timer);
    s_retry_timer = NULL;
  }
}

bool messaging_is_js_ready(void) {
  return s_js_ready;
}

void messaging_request_update(void) {
  if (!s_js_ready) {
    APP_LOG(APP_LOG_LEVEL_WARNING, "JS not ready, cannot request update");
    return;
  }
  
  DictionaryIterator *iter;
  AppMessageResult result = app_message_outbox_begin(&iter);
  
  if (result == APP_MSG_OK) {
    dict_write_int8(iter, KEY_REQUEST_UPDATE, 1);
    result = app_message_outbox_send();
    
    if (result == APP_MSG_OK) {
      APP_LOG(APP_LOG_LEVEL_INFO, "Update request sent");
      s_retry_count = 0;
    } else {
      APP_LOG(APP_LOG_LEVEL_ERROR, "Failed to send update request: %d", result);
      // Schedule retry
      if (s_retry_count < MAX_RETRIES) {
        s_retry_count++;
        s_retry_timer = app_timer_register(RETRY_INTERVAL_MS, retry_timer_callback, NULL);
      }
    }
  } else {
    APP_LOG(APP_LOG_LEVEL_ERROR, "Failed to begin outbox: %d", result);
  }
}

static void retry_timer_callback(void *data) {
  APP_LOG(APP_LOG_LEVEL_INFO, "Retrying update request...");
  messaging_request_update();
}

static void update_timer_callback(void *data) {
  APP_LOG(APP_LOG_LEVEL_INFO, "Periodic update triggered");
  messaging_request_update();
  schedule_next_update();
}

static void schedule_next_update(void) {
  s_update_timer = app_timer_register(s_update_interval_ms, update_timer_callback, NULL);
  APP_LOG(APP_LOG_LEVEL_INFO, "Next update scheduled in %d ms", s_update_interval_ms);
}

void inbox_received_callback(DictionaryIterator *iterator, void *context) {
  // Check for JS Ready signal
  Tuple *ready_tuple = dict_find(iterator, KEY_JS_READY);
  if (ready_tuple) {
    s_js_ready = true;
    APP_LOG(APP_LOG_LEVEL_INFO, "JS is ready!");

    // Cancel any pending retry timer
    if (s_retry_timer) {
      app_timer_cancel(s_retry_timer);
      s_retry_timer = NULL;
    }

    // Request initial data
    messaging_request_update();

    // Schedule periodic updates
    schedule_next_update();

    return;
  }

  // Read train number (now always a string to support UUIDs)
  Tuple *train_tuple = dict_find(iterator, KEY_TRAIN_NUMBER);
  if (train_tuple) {
    strncpy(s_current_train_number, train_tuple->value->cstring, sizeof(s_current_train_number) - 1);
    s_current_train_number[sizeof(s_current_train_number) - 1] = '\0';
    APP_LOG(APP_LOG_LEVEL_INFO, "Train number: %s", s_current_train_number);
  }

  // Read commercial train number
  Tuple *commercial_tuple = dict_find(iterator, KEY_COMMERCIAL_TRAIN_NUMBER);
  if (commercial_tuple) {
    strncpy(s_commercial_train_number, commercial_tuple->value->cstring, sizeof(s_commercial_train_number) - 1);
    s_commercial_train_number[sizeof(s_commercial_train_number) - 1] = '\0';
    APP_LOG(APP_LOG_LEVEL_INFO, "Commercial train number: %s", s_commercial_train_number);
  }

  // Check if we received train data (any of the key train data fields)
  if (train_tuple || commercial_tuple) {
    // Hide loading screen and show main content when train data arrives
    if (s_loading_layer) {
      layer_set_hidden(text_layer_get_layer(s_loading_layer), true);
    }
    // Show the main page (it's hidden initially until data arrives)
    if (s_main_page_container) {
      layer_set_hidden(s_main_page_container, false);
    }
    APP_LOG(APP_LOG_LEVEL_INFO, "Train data received, showing main page");
    
    // Clear time buffers to ensure old values don't persist if not included in update
    s_arrival_time[0] = '\0';
    s_departure_time[0] = '\0';
    s_next_station_arrival[0] = '\0';
    s_next_station_departure[0] = '\0';
  }
  
  // Read station name
  Tuple *station_tuple = dict_find(iterator, KEY_STATION_NAME);
  if (station_tuple) {
    strncpy(s_current_station, station_tuple->value->cstring, sizeof(s_current_station) - 1);
    s_current_station[sizeof(s_current_station) - 1] = '\0';
    APP_LOG(APP_LOG_LEVEL_INFO, "Station: %s", s_current_station);
  }
  
  // Read delay
  Tuple *delay_tuple = dict_find(iterator, KEY_DELAY_MINUTES);
  if (delay_tuple) {
    s_delay_minutes = delay_tuple->value->int32;
    APP_LOG(APP_LOG_LEVEL_INFO, "Delay: %d", s_delay_minutes);
  }
  
  // Read next station (commercial stop)
  Tuple *next_tuple = dict_find(iterator, KEY_NEXT_STATION);
  if (next_tuple) {
    strncpy(s_next_station_name, next_tuple->value->cstring, sizeof(s_next_station_name) - 1);
    s_next_station_name[sizeof(s_next_station_name) - 1] = '\0';
    APP_LOG(APP_LOG_LEVEL_INFO, "Next station: %s", s_next_station_name);
  }
  
  // Read next point (immediate next location, even if non-commercial)
  Tuple *next_point_tuple = dict_find(iterator, KEY_NEXT_POINT);
  if (next_point_tuple) {
    strncpy(s_next_point_name, next_point_tuple->value->cstring, sizeof(s_next_point_name) - 1);
    s_next_point_name[sizeof(s_next_point_name) - 1] = '\0';
    APP_LOG(APP_LOG_LEVEL_INFO, "Next point: %s", s_next_point_name);
  }
  
  // Read arrival time
  Tuple *arrival_tuple = dict_find(iterator, KEY_ARRIVAL_TIME);
  if (arrival_tuple) {
    strncpy(s_arrival_time, arrival_tuple->value->cstring, sizeof(s_arrival_time) - 1);
    s_arrival_time[sizeof(s_arrival_time) - 1] = '\0';
    APP_LOG(APP_LOG_LEVEL_INFO, "Arrival time: %s", s_arrival_time);
  }
  
  // Read departure time
  Tuple *departure_tuple = dict_find(iterator, KEY_DEPARTURE_TIME);
  if (departure_tuple) {
    strncpy(s_departure_time, departure_tuple->value->cstring, sizeof(s_departure_time) - 1);
    s_departure_time[sizeof(s_departure_time) - 1] = '\0';
    APP_LOG(APP_LOG_LEVEL_INFO, "Departure time: %s", s_departure_time);
  }

  // Read arrival delay
  Tuple *arrival_delay_tuple = dict_find(iterator, KEY_ARRIVAL_DELAY);
  if (arrival_delay_tuple) {
    s_arrival_delay = arrival_delay_tuple->value->int32;
    APP_LOG(APP_LOG_LEVEL_INFO, "Arrival delay: %d", s_arrival_delay);
  }

  // Read departure delay
  Tuple *departure_delay_tuple = dict_find(iterator, KEY_DEPARTURE_DELAY);
  if (departure_delay_tuple) {
    s_departure_delay = departure_delay_tuple->value->int32;
    APP_LOG(APP_LOG_LEVEL_INFO, "Departure delay: %d", s_departure_delay);
  }

  // Read next commercial stop data
  Tuple *next_commercial_name_tuple = dict_find(iterator, KEY_NEXT_COMMERCIAL_STOP_NAME);
  if (next_commercial_name_tuple) {
    strncpy(s_next_commercial_stop_name, next_commercial_name_tuple->value->cstring, sizeof(s_next_commercial_stop_name) - 1);
    s_next_commercial_stop_name[sizeof(s_next_commercial_stop_name) - 1] = '\0';
    APP_LOG(APP_LOG_LEVEL_INFO, "Next commercial stop: %s", s_next_commercial_stop_name);
  }

  // Read next station arrival time
  Tuple *next_station_arrival_tuple = dict_find(iterator, KEY_NEXT_STATION_ARRIVAL);
  if (next_station_arrival_tuple) {
    strncpy(s_next_station_arrival, next_station_arrival_tuple->value->cstring, sizeof(s_next_station_arrival) - 1);
    s_next_station_arrival[sizeof(s_next_station_arrival) - 1] = '\0';
    APP_LOG(APP_LOG_LEVEL_INFO, "Next station arrival: %s", s_next_station_arrival);
  }

  // Read next station departure time
  Tuple *next_station_departure_tuple = dict_find(iterator, KEY_NEXT_STATION_DEPARTURE);
  if (next_station_departure_tuple) {
    strncpy(s_next_station_departure, next_station_departure_tuple->value->cstring, sizeof(s_next_station_departure) - 1);
    s_next_station_departure[sizeof(s_next_station_departure) - 1] = '\0';
    APP_LOG(APP_LOG_LEVEL_INFO, "Next station departure: %s", s_next_station_departure);
  }

  // Read current station index
  Tuple *current_index_tuple = dict_find(iterator, KEY_CURRENT_STATION_INDEX);
  if (current_index_tuple) {
    set_current_station_index(current_index_tuple->value->int32);
    APP_LOG(APP_LOG_LEVEL_INFO, "Current station index: %d", s_current_station_index);
  }

  // Read update interval from phone settings
  Tuple *update_interval_tuple = dict_find(iterator, KEY_UPDATE_INTERVAL);
  if (update_interval_tuple) {
    int new_interval = update_interval_tuple->value->int32 * 1000; // Convert seconds to ms
    if (new_interval >= 15000 && new_interval <= 120000 ) { // Validate: 15s to 120s
      s_update_interval_ms = new_interval;
      APP_LOG(APP_LOG_LEVEL_INFO, "Update interval set to %d ms (%d seconds)", s_update_interval_ms, update_interval_tuple->value->int32);
    } else {
      APP_LOG(APP_LOG_LEVEL_WARNING, "Invalid update interval received: %d seconds", update_interval_tuple->value->int32);
    }
  }

  // Handle timetable entry
  Tuple *index_tuple = dict_find(iterator, KEY_TIMETABLE_INDEX);
  if (index_tuple) {
    int index = index_tuple->value->int32;
    
    // Check if this is the start of a new timetable
    if (index == 0) {
      reset_full_timetable();
    }
    
    Tuple *name_tuple = dict_find(iterator, KEY_TIMETABLE_NAME);
    Tuple *arrival_tuple = dict_find(iterator, KEY_TIMETABLE_ARRIVAL);
    Tuple *departure_tuple = dict_find(iterator, KEY_TIMETABLE_DEPARTURE);
    Tuple *arrival_delay_tuple = dict_find(iterator, KEY_TIMETABLE_ARRIVAL_DELAY);
    Tuple *departure_delay_tuple = dict_find(iterator, KEY_TIMETABLE_DEPARTURE_DELAY);
    Tuple *commercial_stop_tuple = dict_find(iterator, KEY_TIMETABLE_COMMERCIAL_STOP);
    
    if (name_tuple) {
      const char *name = name_tuple->value->cstring;
      const char *arrival = arrival_tuple ? arrival_tuple->value->cstring : "";
      const char *departure = departure_tuple ? departure_tuple->value->cstring : "";
      int arrival_delay = arrival_delay_tuple ? arrival_delay_tuple->value->int32 : 0;
      int departure_delay = departure_delay_tuple ? departure_delay_tuple->value->int32 : 0;
      bool commercial_stop = commercial_stop_tuple ? (commercial_stop_tuple->value->int32 != 0) : false;
      
      add_timetable_entry(index, name, arrival, departure, arrival_delay, departure_delay, commercial_stop);
      APP_LOG(APP_LOG_LEVEL_INFO, "Timetable entry %d: %s (commercial: %d)", index, name, commercial_stop);
    }
  }
  
  // Check for timetable complete signal
  Tuple *complete_tuple = dict_find(iterator, KEY_TIMETABLE_COMPLETE);
  if (complete_tuple) {
    s_timetable_loading = false;
    APP_LOG(APP_LOG_LEVEL_INFO, "Timetable complete! Total entries: %d", s_full_timetable_count);
    // Update timetable display when data is complete
    update_timetable();
  }

  // Update display
  update_train_info();
}

void inbox_dropped_callback(AppMessageResult reason, void *context) {
  APP_LOG(APP_LOG_LEVEL_ERROR, "Message dropped! Reason: %d", reason);
}

void outbox_failed_callback(DictionaryIterator *iterator, AppMessageResult reason, void *context) {
  APP_LOG(APP_LOG_LEVEL_ERROR, "Outbox send failed! Reason: %d", reason);
  
  // Retry if this was an update request
  if (s_retry_count < MAX_RETRIES) {
    s_retry_count++;
    s_retry_timer = app_timer_register(RETRY_INTERVAL_MS, retry_timer_callback, NULL);
    APP_LOG(APP_LOG_LEVEL_INFO, "Will retry in %d ms (attempt %d/%d)", RETRY_INTERVAL_MS, s_retry_count, MAX_RETRIES);
  }
}

void outbox_sent_callback(DictionaryIterator *iterator, void *context) {
  APP_LOG(APP_LOG_LEVEL_INFO, "Outbox send success!");
  s_retry_count = 0;
}
