#pragma once

#include "constants.h"

// Main window
extern Window *s_main_window;

// Page containers (for showing/hiding)
extern Layer *s_main_page_container;
extern Layer *s_timetable_page_container;
extern Layer *s_lamp_page_container;

// Window load handler
void main_window_load(Window *window);

// Window unload handler
void main_window_unload(Window *window);
