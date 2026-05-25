// PebbleConduct JavaScript companion
// Handles communication with the phone for real-time train data and config

// Initialize Clay for configuration
var Clay = require('@rebble/clay');
var clayConfig = require('./config');
var clay = new Clay(clayConfig, null, { autoHandleEvents: true });

// Import modular components
var eventHandlers = require('./event-handlers');

// Initialize the application
eventHandlers.init();
