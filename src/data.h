#pragma once

#include "constants.h"

// Timetable entry structure matching API format
typedef struct {
  char name[64];
  char arrival[8];     // HH:MM format
  char departure[8];   // HH:MM format
  int arrival_delay;   // positive = delay, negative = early
  int departure_delay; // positive = delay, negative = early
  bool commercial_stop;
} TimetableEntry;

// App data buffers
extern char s_time_buffer[12];
extern char s_station_buffer[32];
extern char s_status_buffer[64];
extern char s_next_station_buffer[64];

// Current train data
extern char s_current_train_number[64];
extern char s_commercial_train_number[16];
extern char s_current_station[32];
extern int s_delay_minutes;
extern char s_next_station_name[32];
extern char s_next_point_name[32];
extern char s_arrival_time[8];
extern char s_departure_time[8];
extern int s_arrival_delay;
extern int s_departure_delay;

// Next commercial stop data
extern char s_next_commercial_stop_name[24];
extern char s_next_station_arrival[8];
extern char s_next_station_departure[8];

// Current station index in timetable
extern int s_current_station_index;

// Update interval in milliseconds (default: 60 seconds)
extern int s_update_interval_ms;

// Full timetable data from API
extern TimetableEntry s_full_timetable[MAX_TIMETABLE_ENTRIES];
extern int s_full_timetable_count;
extern bool s_timetable_loading;

// Page state
extern int s_current_page;

// Initialize data
void init_data(void);

// Reset full timetable
void reset_full_timetable(void);

// Add entry to full timetable
void add_timetable_entry(int index, const char *name, const char *arrival, const char *departure, 
                         int arrival_delay, int departure_delay, bool commercial_stop);

// Set current station index
void set_current_station_index(int index);
