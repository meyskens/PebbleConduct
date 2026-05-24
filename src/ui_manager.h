#pragma once

#include "constants.h"

// UI Layers - Lamp page
extern Layer *s_lamp_page_container;
extern Layer *s_lamp_bg_layer;
extern bool s_lamp_active;

// UI Layers - Main page
extern TextLayer *s_time_layer;
extern TextLayer *s_train_number_layer;
extern TextLayer *s_station_layer;
extern TextLayer *s_status_layer;
extern TextLayer *s_next_station_layer;
extern TextLayer *s_next_commercial_stop_layer;
extern Layer *s_train_info_bg_layer;
extern Layer *s_next_station_bg_layer;
extern BitmapLayer *s_train_icon_layer;
extern GBitmap *s_train_icon_bitmap;

// UI Layers - Timetable page
extern TextLayer *s_timetable_title_layer;
extern TextLayer *s_timetable_stop_layers[MAX_STOPS];
extern TextLayer *s_timetable_time_layers[MAX_STOPS];  // Time in HH:MM
extern TextLayer *s_timetable_delay_layers[MAX_STOPS]; // Delay (+x/-x)
extern ScrollLayer *s_timetable_scroll_layer; // Scroll layer for timetable
extern Layer *s_timetable_content_layer; // Content container inside scroll
extern BitmapLayer *s_timetable_icon_layers[MAX_STOPS]; // Icon layers for stop type
extern GBitmap *s_train_signal_bitmap;
extern GBitmap *s_train_station_bitmap;

// Status bar
extern StatusBarLayer *s_status_bar;

// Loading layer
extern TextLayer *s_loading_layer;

// Background update proc for train info section
void train_info_bg_update_proc(Layer *layer, GContext *ctx);

// Background update proc for next station section
void next_station_bg_update_proc(Layer *layer, GContext *ctx);

// Background update proc for zebra striping rows
void timetable_row_bg_update_proc(Layer *layer, GContext *ctx);

// Show/hide layers based on current page
void update_page_visibility(void);
