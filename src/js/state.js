// State management for PebbleConduct
// Centralizes all application state

var messageKeys = require('message_keys');

// Current train data state (populated from API)
var trainData = {
  trainNumber: 0,
  stationName: "",
  delayMinutes: 0,
  nextStation: ""
};

// Stored train path data (populated from API)
var storedTrainPath = {
  currentStation: null,
  locations: []
};

// Settings
var appSettings = {
  dataSource: 'tcs',
  trainNumber: '',
  commercialTrainNumber: '',
  date: '',
  tcsUrl: '',
  tcsUsername: '',
  tcsPassword: '',
  dbClientId: '',
  dbApiKey: '',
  nsSubscriptionKey: '',
  updateInterval: '60'
};

// Update timer reference
var updateTimer = null;

module.exports = {
  trainData: trainData,
  storedTrainPath: storedTrainPath,
  appSettings: appSettings,
  updateTimer: updateTimer,
  messageKeys: messageKeys
};
