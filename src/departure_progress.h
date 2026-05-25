#pragma once

#include <pebble.h>

// Progress bar layer
extern Layer *s_departure_progress_layer;

// Initialize the departure progress bar
void departure_progress_init(void);

// Destroy the departure progress bar
void departure_progress_destroy(void);

// Update the progress bar based on current time and departure time
// Returns true if progress bar is visible (countdown active)
bool departure_progress_update(void);

// Check if progress bar should be shown (has both arrival and departure)
bool departure_progress_should_show(void);

// Get the layer for adding to parent
Layer* departure_progress_get_layer(void);
