// Event Handlers module for PebbleConduct
// Handles Pebble events (ready, appmessage, webviewclosed)

var state = require('./state');
var storage = require('./storage');
var scheduler = require('./scheduler');
var apiClients = require('./api-clients');
var messenger = require('./pebble-messenger');

// Pebble app message handler
function setupAppMessageHandler() {
  Pebble.addEventListener('appmessage', function(e) {
    console.log('Received message from Pebble: ' + JSON.stringify(e.payload));
    
    // Handle update requests from the watch
    if (e.payload[state.messageKeys.requestUpdate] === 1) {
      console.log('Update requested by watch');
      apiClients.fetchTrainData();
    }
  });
}

// Pebble ready handler
function setupReadyHandler() {
  Pebble.addEventListener('ready', function(e) {
    console.log('PebbleConduct JS ready');

    // Load saved settings and train path
    storage.loadSettings();

    // Send JS Ready signal to watch
    messenger.sendJsReady();

    // Send update interval to watch
    messenger.sendUpdateInterval();

    // Start periodic updates
    scheduler.startPeriodicUpdates();

    // Immediately fetch and send train data on app start
    console.log('Fetching initial train data on app start...');
    apiClients.fetchTrainData();
  });
}

// Handle Clay settings events
function setupWebviewClosedHandler() {
  Pebble.addEventListener('webviewclosed', function(e) {
    console.log('Clay configuration closed');
    
    if (e.response && e.response !== '') {
      try {
        // Clay returns the settings directly
        var settings = JSON.parse(decodeURIComponent(e.response));
        console.log('Clay settings: ' + JSON.stringify(settings));
        
        // Helper to extract value from Clay's object format
        function getValue(setting) {
          return (setting && typeof setting === 'object' && 'value' in setting) ? setting.value : setting;
        }

        // Update app settings - extract .value from Clay objects
        ['dataSource', 'trainNumber', 'commercialTrainNumber', 'date', 'tcsUrl',
         'tcsUsername', 'tcsPassword', 'dbClientId', 'dbApiKey', 'nsSubscriptionKey', 'updateInterval'
        ].forEach(function(key) {
          if (settings[key] !== undefined) state.appSettings[key] = getValue(settings[key]);
        });

        // Update train data for display
        var trainNumberValue = getValue(settings.trainNumber);
        if (trainNumberValue) {
          var parsed = parseInt(trainNumberValue);
          state.trainData.trainNumber = (!isNaN(parsed) && parsed.toString() === trainNumberValue) ? parsed : 0;
        }
        
        // Save to localStorage (save appSettings with extracted values, not raw Clay settings)
        storage.saveSettings();
        console.log('Settings saved to localStorage');
        
        // Restart scheduler with new interval
        scheduler.startPeriodicUpdates();

        // Send update interval to watch
        messenger.sendUpdateInterval();

        // Fetch new data with updated settings
        apiClients.fetchTrainData();
      } catch (err) {
        console.log('Error handling Clay settings: ' + err);
      }
    }
  });
}

// Initialize all event handlers
function init() {
  setupReadyHandler();
  setupAppMessageHandler();
  setupWebviewClosedHandler();
}

module.exports = {
  init: init
};
