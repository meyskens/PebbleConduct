#include "data.h"

// App data buffers
char s_time_buffer[12];
char s_station_buffer[32];
char s_status_buffer[64];
char s_next_station_buffer[64];

// Current train data (initialized empty, populated from API)
char s_current_train_number[64] = "";
char s_commercial_train_number[16] = "";
char s_current_station[32] = "";
int s_delay_minutes = 0;
char s_next_station_name[32] = "";
char s_next_point_name[32] = "";
char s_arrival_time[8] = "";
char s_departure_time[8] = "";
int s_arrival_delay = 0;
int s_departure_delay = 0;

// Next commercial stop data
char s_next_commercial_stop_name[24] = "";
char s_next_station_arrival[8] = "";
char s_next_station_departure[8] = "";
int s_next_station_arrival_delay = DELAY_NO_INFO;
int s_next_station_departure_delay = DELAY_NO_INFO;

// Current station index in timetable
int s_current_station_index = 0;

// Update interval in milliseconds (default: 60 seconds)
int s_update_interval_ms = 60000;

// Full timetable data from API
TimetableEntry s_full_timetable[MAX_TIMETABLE_ENTRIES];
int s_full_timetable_count = 0;
bool s_timetable_loading = false;

// Page state
int s_current_page = PAGE_MAIN;

void init_data(void) {
  // Data is initialized at startup
}

void reset_full_timetable(void) {
  s_full_timetable_count = 0;
  s_timetable_loading = true;
}

void add_timetable_entry(int index, const char *name, const char *arrival, const char *departure, 
                         int arrival_delay, int departure_delay, bool commercial_stop) {
  if (index >= 0 && index < MAX_TIMETABLE_ENTRIES) {
    // Use snprintf for safer string handling with truncation warning
    int name_len = snprintf(s_full_timetable[index].name, sizeof(s_full_timetable[index].name), "%s", name);
    if (name_len >= (int)sizeof(s_full_timetable[index].name)) {
      APP_LOG(APP_LOG_LEVEL_WARNING, "Station name truncated: %s", name);
    }
    
    int arr_len = snprintf(s_full_timetable[index].arrival, sizeof(s_full_timetable[index].arrival), "%s", arrival);
    if (arr_len >= (int)sizeof(s_full_timetable[index].arrival)) {
      APP_LOG(APP_LOG_LEVEL_WARNING, "Arrival time truncated: %s", arrival);
    }
    
    int dep_len = snprintf(s_full_timetable[index].departure, sizeof(s_full_timetable[index].departure), "%s", departure);
    if (dep_len >= (int)sizeof(s_full_timetable[index].departure)) {
      APP_LOG(APP_LOG_LEVEL_WARNING, "Departure time truncated: %s", departure);
    }
    
    s_full_timetable[index].arrival_delay = arrival_delay;
    s_full_timetable[index].departure_delay = departure_delay;
    s_full_timetable[index].commercial_stop = commercial_stop;
    
    if (index >= s_full_timetable_count) {
      s_full_timetable_count = index + 1;
    }
  }
}

void set_current_station_index(int index) {
  s_current_station_index = index;
}
