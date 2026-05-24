// Scheduler module for PebbleConduct
// Handles periodic update scheduling

var state = require('./state');
var apiClients = require('./api-clients');

// Default update interval in milliseconds
var DEFAULT_INTERVAL = 60000;

// Schedule periodic updates
function startPeriodicUpdates() {
  if (state.updateTimer) {
    clearInterval(state.updateTimer);
  }
  
  // Get interval from settings (convert seconds to milliseconds)
  var intervalSeconds = parseInt(state.appSettings.updateInterval, 10) || 60;
  var intervalMs = intervalSeconds * 1000;
  
  state.updateTimer = setInterval(function() {
    console.log('Periodic update triggered');
    apiClients.fetchTrainData();
  }, intervalMs);
  
  console.log('Periodic updates started (every ' + intervalSeconds + ' seconds)');
}

// Stop periodic updates
function stopPeriodicUpdates() {
  if (state.updateTimer) {
    clearInterval(state.updateTimer);
    state.updateTimer = null;
    console.log('Periodic updates stopped');
  }
}

// Cleanup on app exit
Pebble.addEventListener('appmessage', function(e) {
  if (e.payload && e.payload.stop === 1) {
    stopPeriodicUpdates();
  }
});

module.exports = {
  startPeriodicUpdates: startPeriodicUpdates,
  stopPeriodicUpdates: stopPeriodicUpdates
};
