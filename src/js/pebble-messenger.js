// Pebble Messenger module for PebbleConduct
// Handles all communication with the Pebble watch

var state = require('./state');

// Send train data to Pebble
function sendTrainDataToPebble() {
  var trainData = state.trainData;
  var storedTrainPath = state.storedTrainPath;
  var appSettings = state.appSettings;
  var messageKeys = state.messageKeys;
  
  // Calculate next point and next commercial station from the timetable
  var nextPointName = '';
  var nextCommercialStationName = '';
  var arrivalTime = '';
  var departureTime = '';
  var arrivalDelay = -999;
  var departureDelay = -999;
  
  if (storedTrainPath.currentStation && storedTrainPath.locations.length > 0) {
    var currentIdx = storedTrainPath.currentStation.id || 0;
    
    // Find next point (immediate next location)
    if (storedTrainPath.locations[currentIdx + 1]) {
      nextPointName = storedTrainPath.locations[currentIdx + 1].name;
    }
    
    // Find next commercial stop
    for (var i = currentIdx + 1; i < storedTrainPath.locations.length; i++) {
      var loc = storedTrainPath.locations[i];
      if (loc.commercialStop !== false) {
        nextCommercialStationName = loc.name;
        break;
      }
    }
    
    // Get arrival/departure times and delays for current station
    arrivalTime = storedTrainPath.currentStation.arrival || '';
    departureTime = storedTrainPath.currentStation.departure || '';
    arrivalDelay = storedTrainPath.currentStation.arrivalDelay != null ? storedTrainPath.currentStation.arrivalDelay : -999;
    departureDelay = storedTrainPath.currentStation.departureDelay != null ? storedTrainPath.currentStation.departureDelay : -999;
  }
  
  // Use configured commercial train number if available, otherwise use train number
  var commercialNumber = appSettings.commercialTrainNumber;
  if (!commercialNumber && trainData.trainNumber) {
    commercialNumber = trainData.trainNumber.toString();
  }
  if (!commercialNumber) {
    commercialNumber = "Unknown";
  }
  
  var message = {};
  var trainNumberStr = appSettings.trainNumber || '';
  message[messageKeys.trainNumber] = trainNumberStr;
  message[messageKeys.commercialTrainNumber] = commercialNumber;
  message[messageKeys.stationName] = trainData.stationName;
  message[messageKeys.delayMinutes] = trainData.delayMinutes;
  message[messageKeys.nextStation] = nextCommercialStationName || '';
  message[messageKeys.nextPoint] = nextPointName || '';
  
  // Add current station index if available
  if (storedTrainPath.currentStation && storedTrainPath.currentStation.id !== undefined) {
    message[messageKeys.currentStationIndex] = storedTrainPath.currentStation.id;
  }
  
  // Add arrival/departure times if available
  if (arrivalTime) {
    message[messageKeys.arrivalTime] = arrivalTime;
  }
  if (departureTime) {
    message[messageKeys.departureTime] = departureTime;
  }

  // Add arrival/departure delays
  console.log('messageKeys object: ' + JSON.stringify(messageKeys));
  console.log('arrivalDelay key value: ' + messageKeys.arrivalDelay);
  console.log('departureDelay key value: ' + messageKeys.departureDelay);
  console.log('Sending delays - arrivalDelay key: ' + messageKeys.arrivalDelay + ', value: ' + arrivalDelay);
  console.log('Sending delays - departureDelay key: ' + messageKeys.departureDelay + ', value: ' + departureDelay);
  if (messageKeys.arrivalDelay !== undefined) {
    message[messageKeys.arrivalDelay] = arrivalDelay;
  } else {
    console.log('ERROR: messageKeys.arrivalDelay is undefined!');
  }
  if (messageKeys.departureDelay !== undefined) {
    message[messageKeys.departureDelay] = departureDelay;
  } else {
    console.log('ERROR: messageKeys.departureDelay is undefined!');
  }

  // Add next commercial stop data if available
  var nextStationArrival = '';
  var nextStationDeparture = '';
  var nextStationArrivalDelay = -999;
  var nextStationDepartureDelay = -999;
  if (storedTrainPath.currentStation && storedTrainPath.locations.length > 0) {
    var currentIdx = storedTrainPath.currentStation.id || 0;

    // Find next commercial stop
    for (var i = currentIdx + 1; i < storedTrainPath.locations.length; i++) {
      var loc = storedTrainPath.locations[i];
      if (loc.commercialStop === true) {
        message[messageKeys.nextCommercialStopName] = loc.name;
        nextStationArrival = loc.arrival || '';
        nextStationDeparture = loc.departure || '';
        nextStationArrivalDelay = loc.arrivalDelay != null ? loc.arrivalDelay : -999;
        nextStationDepartureDelay = loc.departureDelay != null ? loc.departureDelay : -999;
        break;
      }
    }
  }

  // Add next station arrival/departure times
  if (nextStationArrival) {
    message[messageKeys.nextStationArrival] = nextStationArrival;
  }
  if (nextStationDeparture) {
    message[messageKeys.nextStationDeparture] = nextStationDeparture;
  }
  // Add next station delays
  console.log('Next station delays - arrivalDelay key: ' + messageKeys.nextStationArrivalDelay + ', value: ' + nextStationArrivalDelay);
  console.log('Next station delays - departureDelay key: ' + messageKeys.nextStationDepartureDelay + ', value: ' + nextStationDepartureDelay);
  if (messageKeys.nextStationArrivalDelay !== undefined) {
    message[messageKeys.nextStationArrivalDelay] = nextStationArrivalDelay;
  } else {
    console.log('ERROR: messageKeys.nextStationArrivalDelay is undefined!');
  }
  if (messageKeys.nextStationDepartureDelay !== undefined) {
    message[messageKeys.nextStationDepartureDelay] = nextStationDepartureDelay;
  } else {
    console.log('ERROR: messageKeys.nextStationDepartureDelay is undefined!');
  }

  Pebble.sendAppMessage(message, 
    function(e) {
      console.log('Train data sent successfully');
    },
    function(e) {
      console.log('Error sending train data: ' + e.error.message);
    }
  );
}

// Send timetable to Pebble in chunks
function sendTimetableToPebble() {
  var storedTrainPath = state.storedTrainPath;
  var messageKeys = state.messageKeys;
  var locations = storedTrainPath.locations;
  
  if (!locations || locations.length === 0) {
    console.log('No timetable data to send');
    return;
  }
  
  // Calculate filtered indices: 5 previous + current +25 next
  var currentIdx = storedTrainPath.currentStation ? (storedTrainPath.currentStation.id || 0) : 0;
  var startIdx = Math.max(0, currentIdx - 5);  // 5 previous
  var endIdx = Math.min(locations.length - 1, currentIdx + 25);  // 25 next
  
  // Find the next commercial stop from current position
  var nextCommercialStopIdx = -1;
  for (var i = currentIdx + 1; i < locations.length; i++) {
    if (locations[i].commercialStop === true) {
      nextCommercialStopIdx = i;
      break;
    }
  }
  
  // Create filtered array
  var filteredLocations = [];
  for (var i = startIdx; i <= endIdx; i++) {
    filteredLocations.push(locations[i]);
  }
  
  // Include next commercial stop if it's beyond the normal filter range
  if (nextCommercialStopIdx > endIdx) {
    filteredLocations.push(locations[nextCommercialStopIdx]);
    console.log('Added next commercial stop at index ' + nextCommercialStopIdx + 
                ' (beyond normal filter range)');
  }
  
  console.log('Sending filtered timetable: ' + filteredLocations.length + ' entries (from ' + 
              locations.length + ' total), indices ' + startIdx + ' to ' + endIdx + 
              (nextCommercialStopIdx > endIdx ? ' + commercial stop at ' + nextCommercialStopIdx : ''));
  
  var index = 0;
  
  function sendNextEntry() {
    if (index >= filteredLocations.length) {
      // Send completion signal
      var completeMessage = {};
      completeMessage[messageKeys.timetableComplete] = 1;
      Pebble.sendAppMessage(completeMessage, function() {
        console.log('Timetable transmission complete');
      }, function(e) {
        console.log('Error sending completion: ' + e.error.message);
      });
      return;
    }
    
    var entry = filteredLocations[index];
    var message = {};
    message[messageKeys.timetableIndex] = index;
    message[messageKeys.timetableName] = entry.name.substring(0, 63); // Limit length
    message[messageKeys.timetableArrival] = entry.arrival || '';
    message[messageKeys.timetableDeparture] = entry.departure || '';
    // Use -999 as sentinel for "no delay info" (null/undefined from API)
    message[messageKeys.timetableArrivalDelay] = entry.arrivalDelay != null ? entry.arrivalDelay : -999;
    message[messageKeys.timetableDepartureDelay] = entry.departureDelay != null ? entry.departureDelay : -999;
    message[messageKeys.timetableCommercialStop] = entry.commercialStop ? 1 : 0;
    
    Pebble.sendAppMessage(message,
      function() {
        index++;
        sendNextEntry();
      },
      function(e) {
        console.log('Error sending timetable entry ' + index + ': ' + e.error.message);
        // Retry this entry
        setTimeout(sendNextEntry, 500);
      }
    );
  }
  
  sendNextEntry();
}

// Send JS Ready signal to watch
function sendJsReady() {
  var message = {};
  message[state.messageKeys.JSReady] = 1;
  Pebble.sendAppMessage(message, function() {
    console.log('JS Ready signal sent');
  }, function(e) {
    console.log('Error sending JS Ready: ' + e.error.message);
  });
}

// Send update interval to watch
function sendUpdateInterval() {
  var intervalSeconds = parseInt(state.appSettings.updateInterval, 10) || 60;
  var message = {};
  message[state.messageKeys.updateInterval] = intervalSeconds;
  Pebble.sendAppMessage(message, function() {
    console.log('Update interval sent: ' + intervalSeconds + ' seconds');
  }, function(e) {
    console.log('Error sending update interval: ' + e.error.message);
  });
}

module.exports = {
  sendTrainDataToPebble: sendTrainDataToPebble,
  sendTimetableToPebble: sendTimetableToPebble,
  sendJsReady: sendJsReady,
  sendUpdateInterval: sendUpdateInterval
};
