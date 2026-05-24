// API Clients module for PebbleConduct
// Thin wrapper that delegates to individual API client modules

var state = require('./state');
var tcsClient = require('./api/tcs-client');
var whereistheesClient = require('./api/whereisthees-client');
var dbRisClient = require('./api/db-ris-client');
var nsClient = require('./api/ns-client');

// Re-export individual API client functions
function fetchFromTCS() {
  return tcsClient.fetchFromTCS();
}

function fetchFromWhereIsTheEs() {
  return whereistheesClient.fetchFromWhereIsTheEs();
}

function fetchFromDBRIS() {
  return dbRisClient.fetchFromDBRIS();
}

function fetchFromNS() {
  return nsClient.fetchFromNS();
}

// Fetch train data from the configured API
function fetchTrainData() {
  console.log('Fetching train data from ' + state.appSettings.dataSource + '...');

  switch (state.appSettings.dataSource) {
    case 'tcs':
      fetchFromTCS();
      break;
    case 'whereisthees':
      fetchFromWhereIsTheEs();
      break;
    case 'db_ris':
      fetchFromDBRIS();
      break;
    case 'ns':
      fetchFromNS();
      break;
    default:
      console.log('Unknown data source: ' + state.appSettings.dataSource);
      // Send cached data if available
      var messenger = require('./pebble-messenger');
      messenger.sendTrainDataToPebble();
      messenger.sendTimetableToPebble();
  }
}

module.exports = {
  fetchFromTCS: fetchFromTCS,
  fetchFromWhereIsTheEs: fetchFromWhereIsTheEs,
  fetchFromDBRIS: fetchFromDBRIS,
  fetchFromNS: fetchFromNS,
  fetchTrainData: fetchTrainData
};
