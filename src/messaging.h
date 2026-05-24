#pragma once

#include "constants.h"

// Initialize messaging system
void messaging_init(void);

// Deinitialize messaging system (cancel timers)
void messaging_deinit(void);

// Check if JS is ready
bool messaging_is_js_ready(void);

// Request data update from phone
void messaging_request_update(void);

// AppMessage inbox received handler
void inbox_received_callback(DictionaryIterator *iterator, void *context);

// AppMessage inbox dropped handler
void inbox_dropped_callback(AppMessageResult reason, void *context);

// AppMessage outbox failed handler
void outbox_failed_callback(DictionaryIterator *iterator, AppMessageResult reason, void *context);

// AppMessage outbox sent handler
void outbox_sent_callback(DictionaryIterator *iterator, void *context);
