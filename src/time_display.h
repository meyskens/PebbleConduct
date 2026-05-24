#pragma once

#include "constants.h"

// Update the time display
void update_time(void);

// Tick handler for time updates
void tick_handler(struct tm *tick_time, TimeUnits units_changed);
