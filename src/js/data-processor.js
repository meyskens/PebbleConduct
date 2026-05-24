// Data Processor module for PebbleConduct
// Transforms API responses into internal format

var state = require('./state');
var storage = require('./storage');
var messenger = require('./pebble-messenger');

// Process TCS Raildesk data into internal format
function processTCSData(data) {
  console.log('Processing TCS Raildesk data');

  if (!data || !data.locations || !Array.isArray(data.locations)) {
    console.log('Invalid TCS data format');
    return;
  }

  // Transform TCS locations to internal format
  var locations = data.locations.map(function(loc, idx) {
    return {
      name: loc.name || 'Unknown',
      arrival: loc.arrival || '',
      departure: loc.departure || '',
      arrivalDelay: loc.arrivalDelay != null ? loc.arrivalDelay : -999,
      departureDelay: loc.departureDelay != null ? loc.departureDelay : -999,
      commercialStop: loc.commercialStop || false
    };
  });

  // Find current station by matching name from currentStation
  var currentStation = null;
  var currentIdx = 0;

  // Use currentStation.name to find the matching location
  var targetName = (data.currentStation && data.currentStation.name) 
    ? data.currentStation.name 
    : null;

  if (targetName) {
    // Find the location with matching name
    for (var i = 0; i < data.locations.length; i++) {
      if (data.locations[i].name === targetName) {
        currentIdx = i;
        currentStation = {
          id: i,
          name: data.locations[i].name || 'Unknown',
          arrival: data.locations[i].arrival || '',
          departure: data.locations[i].departure || '',
          arrivalDelay: data.locations[i].arrivalDelay != null ? data.locations[i].arrivalDelay : -999,
          departureDelay: data.locations[i].departureDelay != null ? data.locations[i].departureDelay : -999
        };
        break;
      }
    }
  }

  // If no current station found, default to first
  if (!currentStation && locations.length > 0) {
    currentStation = {
      id: 0,
      name: locations[0].name,
      arrival: locations[0].arrival,
      departure: locations[0].departure,
      arrivalDelay: locations[0].arrivalDelay,
      departureDelay: locations[0].departureDelay
    };
  }

  state.storedTrainPath = {
    currentStation: currentStation,
    locations: locations
  };

  // Update train data for display
  if (currentStation) {
    state.trainData.stationName = currentStation.name;
    state.trainData.delayMinutes = currentStation.arrivalDelay || currentStation.departureDelay || 0;

    // Find next commercial stop
    var nextStationName = '';
    for (var j = currentIdx + 1; j < locations.length; j++) {
      if (data.locations[j] && (data.locations[j].commercialStop === true || data.locations[j].commercialStop === 'true' || data.locations[j].commercialStop === 1 || data.locations[j].commercialStop === '1')) {
        nextStationName = locations[j].name;
        break;
      }
    }
    // Fallback to next location if no commercial stop
    if (!nextStationName && locations[currentIdx + 1]) {
      nextStationName = locations[currentIdx + 1].name;
    }
    state.trainData.nextStation = nextStationName || 'Terminus';
  }

  // Save to localStorage
  storage.saveTrainPath();

  // Send to watch
  messenger.sendTrainDataToPebble();
  messenger.sendTimetableToPebble();
}

// Process train data from API and update stored path
function processTrainData(data) {
  console.log('Processing train data');
  
  // Transform API response to internal format
  if (data.currentStation) {
    state.storedTrainPath.currentStation = {
      id: data.currentStation.id || 0,
      name: data.currentStation.name || 'Unknown',
      arrival: data.currentStation.arrival || '',
      departure: data.currentStation.departure || '',
      arrivalDelay: data.currentStation.arrivalDelay != null ? data.currentStation.arrivalDelay : -999,
      departureDelay: data.currentStation.departureDelay != null ? data.currentStation.departureDelay : -999
    };
  }
  
  if (data.locations && Array.isArray(data.locations)) {
    state.storedTrainPath.locations = data.locations.map(function(loc, idx) {
      return {
        name: loc.name || 'Unknown',
        arrival: loc.arrival || '',
        departure: loc.departure || '',
        arrivalDelay: loc.arrivalDelay != null ? loc.arrivalDelay : -999,
        departureDelay: loc.departureDelay != null ? loc.departureDelay : -999,
        commercialStop: loc.commercialStop || false
      };
    });
  }
  
  // Update current train data for display
  if (state.storedTrainPath.currentStation) {
    state.trainData.stationName = state.storedTrainPath.currentStation.name;
    state.trainData.delayMinutes = state.storedTrainPath.currentStation.arrivalDelay || 
                             state.storedTrainPath.currentStation.departureDelay || 0;
  }
  
  // Find next station
  if (state.storedTrainPath.locations.length > 0 && state.storedTrainPath.currentStation) {
    var currentIdx = state.storedTrainPath.currentStation.id || 0;
    var nextStation = state.storedTrainPath.locations[currentIdx + 1];
    if (nextStation) {
      state.trainData.nextStation = nextStation.name;
    }
  }
  
  // Save to localStorage
  storage.saveTrainPath();
  
  // Send to watch
  messenger.sendTrainDataToPebble();
  messenger.sendTimetableToPebble();
}

module.exports = {
  processTCSData: processTCSData,
  processTrainData: processTrainData
};
