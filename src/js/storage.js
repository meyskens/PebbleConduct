var state = require('./state');

var STORAGE_KEYS = {
  SETTINGS: 'pebbleconduct_settings',
  TRAIN_PATH: 'pebbleconduct_trainpath'
};

function getValue(setting) {
  return (setting && typeof setting === 'object' && 'value' in setting) ? setting.value : setting;
}

function loadSettings() {
  var savedSettings = localStorage.getItem(STORAGE_KEYS.SETTINGS);
  if (savedSettings) {
    try {
      var settings = JSON.parse(savedSettings);

      ['dataSource', 'trainNumber', 'commercialTrainNumber', 'date',
       'tcsUrl', 'tcsUsername', 'tcsPassword', 'dbClientId', 'dbApiKey', 'nsSubscriptionKey']
        .forEach(function(key) {
          if (settings[key]) state.appSettings[key] = getValue(settings[key]);
        });

      var trainNumber = getValue(settings.trainNumber);
      if (trainNumber) {
        var parsed = parseInt(trainNumber);
        state.trainData.trainNumber = (!isNaN(parsed) && parsed.toString() === trainNumber) ? parsed : 0;
      }

      console.log('Loaded saved settings: ' + JSON.stringify(state.appSettings));
    } catch (e) {
      console.log('Error loading settings: ' + e);
    }
  }

  var savedPath = localStorage.getItem(STORAGE_KEYS.TRAIN_PATH);
  if (savedPath) {
    try {
      state.storedTrainPath = JSON.parse(savedPath);
      console.log('Loaded stored train path with ' + state.storedTrainPath.locations.length + ' locations');
    } catch (e) {
      console.log('Error loading train path: ' + e);
    }
  }
}

function saveSettings() {
  localStorage.setItem(STORAGE_KEYS.SETTINGS, JSON.stringify(state.appSettings));
}

function saveTrainPath() {
  localStorage.setItem(STORAGE_KEYS.TRAIN_PATH, JSON.stringify(state.storedTrainPath));
}

module.exports = {
  loadSettings: loadSettings,
  saveSettings: saveSettings,
  saveTrainPath: saveTrainPath,
  getValue: getValue
};
