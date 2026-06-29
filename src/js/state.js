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

// Settings - defaults must match Clay configuration defaults
var appSettings = {
  dataSource: 'whereisthees',
  trainNumber: '452',
  commercialTrainNumber: 'ES452',
  date: '',
  tcsUrl: '',
  tcsAuthMethod: 'credentials',
  tcsUsername: '',
  tcsPassword: '',
  tcsAspNetCookieC1: '',
  tcsAspNetCookieC2: '',
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
