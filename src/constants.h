#pragma once

#include <pebble.h>

// Page navigation
#define PAGE_MAIN 0
#define PAGE_TIMETABLE 1
#define PAGE_LAMP 2
#define MAX_STOPS 32  // Max entries to display (JS sends up to 31: 5 prev + current + 25 next)

// AppMessage keys (must match appinfo.json)
enum {
  KEY_TRAIN_NUMBER = 0,
  KEY_STATION_NAME = 1,
  KEY_DELAY_MINUTES = 3,
  KEY_NEXT_STATION = 4,
  KEY_JS_READY = 6,
  KEY_REQUEST_UPDATE = 7,
  KEY_TIMETABLE_INDEX = 8,
  KEY_TIMETABLE_NAME = 9,
  KEY_TIMETABLE_ARRIVAL = 10,
  KEY_TIMETABLE_DEPARTURE = 11,
  KEY_TIMETABLE_ARRIVAL_DELAY = 12,
  KEY_TIMETABLE_DEPARTURE_DELAY = 13,
  KEY_TIMETABLE_COMPLETE = 14,
  KEY_CURRENT_STATION_INDEX = 15,
  KEY_COMMERCIAL_TRAIN_NUMBER = 16,
  KEY_ARRIVAL_TIME = 17,
  KEY_DEPARTURE_TIME = 18,
  KEY_NEXT_POINT = 19,
  KEY_TIMETABLE_COMMERCIAL_STOP = 20,
  KEY_NEXT_COMMERCIAL_STOP_NAME = 21,
  KEY_NEXT_STATION_ARRIVAL = 22,
  KEY_NEXT_STATION_DEPARTURE = 23,
  KEY_UPDATE_INTERVAL = 24,
  KEY_ARRIVAL_DELAY = 25,
  KEY_DEPARTURE_DELAY = 26,
  KEY_NEXT_STATION_ARRIVAL_DELAY = 27,
  KEY_NEXT_STATION_DEPARTURE_DELAY = 28
};

// Maximum timetable entries (reduced for aplite memory constraints)
#define MAX_TIMETABLE_ENTRIES 32

// Sentinel value for "no delay information" (null from API)
#define DELAY_NO_INFO -999

// Screen size detection for layout optimization
// Standard Pebble screens: 144x168 (aplite, basalt, diorite)
// Large screens: 200x228 (emery, flint), 260x260 (gabbro)
// Round screens: 180x180 (chalk), 260x260 (gabbro)
#if (PBL_DISPLAY_WIDTH >= 200 && PBL_DISPLAY_HEIGHT >= 228) || (PBL_DISPLAY_WIDTH == 260 && PBL_DISPLAY_HEIGHT == 260)
  #define IS_LARGE_SCREEN 1
#else
  #define IS_LARGE_SCREEN 0
#endif

// Round screen detection - both Chalk (180x180) and Gabbro (260x260) are round
#if (PBL_DISPLAY_WIDTH == 180 && PBL_DISPLAY_HEIGHT == 180) || (PBL_DISPLAY_WIDTH == 260 && PBL_DISPLAY_HEIGHT == 260)
  #define IS_ROUND_SCREEN 1
#else
  #define IS_ROUND_SCREEN 0
#endif

// Round screen padding values (for Chalk and Gabbro platforms)
// These values account for the rounded corners of the display
// Gabbro (260x260) has more screen real estate but still needs padding for corners
#if IS_ROUND_SCREEN
  #if PBL_DISPLAY_WIDTH == 260
    // Gabbro has larger screen but still needs corner padding
    #define ROUND_PADDING_LARGE 25   // For timetable and next station sections
    #define ROUND_PADDING_SMALL 15   // For current station
    #define ROUND_PADDING_DEFAULT 20 // General padding
  #else
    // Chalk (180x180) - original values
    #define ROUND_PADDING_LARGE 20   // For timetable and next station sections
    #define ROUND_PADDING_SMALL 10   // For current station
    #define ROUND_PADDING_DEFAULT 15 // General padding
  #endif
#else
  #define ROUND_PADDING_LARGE 0
  #define ROUND_PADDING_SMALL 0
  #define ROUND_PADDING_DEFAULT 0
#endif
