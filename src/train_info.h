#pragma once

#include "constants.h"

// Format time offset string (+min, -min, or +0)
void format_time_offset(char *buffer, size_t size, int delay_minutes);

// Get color for delay (green for +0/+1/negative, red for +2 or more)
GColor get_delay_color(int delay_minutes);

// Update train information display
void update_train_info(void);
