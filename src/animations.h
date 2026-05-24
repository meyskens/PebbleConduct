#pragma once

#include <pebble.h>

// Initialize animation system
void init_animations(void);

// Set animation container layers
void set_animation_containers(Layer *main_container, Layer *timetable_container, Layer *lamp_container);

// Lamp animation
void start_lamp_animation(void);
void stop_lamp_animation(void);
bool is_lamp_active(void);

// Animate to a specific page with slide transition
void animate_to_page(int target_page);

// Check if animation is currently running
bool is_animation_running(void);
