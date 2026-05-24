#pragma once

#include "constants.h"

// Back button click handler - go back or turn off lamp
void back_click_handler(ClickRecognizerRef recognizer, void *context);

// Up button click handler - go to previous page
void up_click_handler(ClickRecognizerRef recognizer, void *context);

// Down button click handler - go to next page
void down_click_handler(ClickRecognizerRef recognizer, void *context);

// Click config provider
void click_config_provider(void *context);
