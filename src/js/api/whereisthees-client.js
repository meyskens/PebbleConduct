// whereisthe.es API Client
// Handles fetching data from whereisthe.es API

var state = require('../state');
var dataProcessor = require('../data-processor');
var timeUtils = require('../time-utils');
var helpers = require('./helpers');

// Convert whereisthe.es response to TCS format
function transformWhereIsTheEsToTCS(data) {
  if (!data || !data.Stops || !Array.isArray(data.Stops)) {
    console.log('Invalid whereisthe.es data format');
    return null;
  }



  // Transform stops to TCS locations format
  var locations = data.Stops.map(function(stop, idx) {
    var hasArrival = helpers.isValidTime(stop.ArrivalTime);
    var hasDeparture = helpers.isValidTime(stop.DepartureTime);
    var hasRealArrival = helpers.isValidTime(stop.RealArrivalTime);
    var hasRealDeparture = helpers.isValidTime(stop.RealDepartureTime);

    // Use real-time data if available, otherwise use scheduled
    var arrival = hasRealArrival ? stop.RealArrivalTime : (hasArrival ? stop.ArrivalTime : '');
    var departure = hasRealDeparture ? stop.RealDepartureTime : (hasDeparture ? stop.DepartureTime : '');

    // Calculate delays
    var arrivalDelay = hasRealArrival && hasArrival ? helpers.calculateDelay(stop.ArrivalTime, stop.RealArrivalTime) : 0;
    var departureDelay = hasRealDeparture && hasDeparture ? helpers.calculateDelay(stop.DepartureTime, stop.RealDepartureTime) : 0;

    return {
      name: stop.StationName,
      arrival: timeUtils.utcToLocalTime(arrival),
      departure: timeUtils.utcToLocalTime(departure),
      arrivalDelay: arrivalDelay,
      departureDelay: departureDelay,
      commercialStop: true // All stops in whereisthe.es are commercial stops
    };
  });

  // Find current station based on real-time data
  // Current station is the last one with real-time data that has passed
  var currentStation = null;
  var now = new Date();

  for (var i = 0; i < data.Stops.length; i++) {
    var stop = data.Stops[i];
    var hasRealDeparture = helpers.isValidTime(stop.RealDepartureTime);
    var hasRealArrival = helpers.isValidTime(stop.RealArrivalTime);

    // Check if train has departed this station
    if (hasRealDeparture) {
      var departureTime = new Date(stop.RealDepartureTime);
      if (departureTime < now) {
        // Train has left this station, move to next
        continue;
      }
    }

    // This is the current station - train hasn't departed yet
    var arrivalTime = hasRealArrival ? stop.RealArrivalTime : (helpers.isValidTime(stop.ArrivalTime) ? stop.ArrivalTime : '');
    var departureTime = hasRealDeparture ? stop.RealDepartureTime : (helpers.isValidTime(stop.DepartureTime) ? stop.DepartureTime : '');
    currentStation = {
      id: i,
      name: stop.StationName,
      arrival: timeUtils.utcToLocalTime(arrivalTime),
      departure: timeUtils.utcToLocalTime(departureTime),
      arrivalDelay: hasRealArrival && helpers.isValidTime(stop.ArrivalTime) ? helpers.calculateDelay(stop.ArrivalTime, stop.RealArrivalTime) : 0,
      departureDelay: hasRealDeparture && helpers.isValidTime(stop.DepartureTime) ? helpers.calculateDelay(stop.DepartureTime, stop.RealDepartureTime) : 0
    };
    break;
  }

  // If no current station found (train hasn't started or all stops passed), default to first or last
  if (!currentStation && locations.length > 0) {
    // Check if train has started
    var firstStop = data.Stops[0];
    var hasStarted = helpers.isValidTime(firstStop.RealDepartureTime);

    if (!hasStarted) {
      // Train hasn't started yet
      currentStation = {
        id: 0,
        name: locations[0].name,
        arrival: timeUtils.utcToLocalTime(locations[0].arrival),
        departure: timeUtils.utcToLocalTime(locations[0].departure),
        arrivalDelay: locations[0].arrivalDelay,
        departureDelay: locations[0].departureDelay
      };
    } else {
      // Train has completed journey
      var lastIdx = locations.length - 1;
      currentStation = {
        id: lastIdx,
        name: locations[lastIdx].name,
        arrival: timeUtils.utcToLocalTime(locations[lastIdx].arrival),
        departure: timeUtils.utcToLocalTime(locations[lastIdx].departure),
        arrivalDelay: locations[lastIdx].arrivalDelay,
        departureDelay: locations[lastIdx].departureDelay
      };
    }
  }

  return {
    locations: locations,
    currentStation: currentStation
  };
}

// Fetch train data from whereisthe.es API
function fetchFromWhereIsTheEs() {
  console.log('Fetching from whereisthe.es...');

  if (!state.appSettings.trainNumber) {
    console.log('Train number not configured');
    return;
  }

  // Use configured date or today's date
  var date = state.appSettings.date || helpers.getTodayDate();

  // whereisthe.es API endpoint: /api/v1/timetable/{date}/{trainNumber}
  var url = 'https://whereisthe.es/api/v1/timetable/' + date + '/' + state.appSettings.trainNumber;
  console.log('whereisthe.es URL: ' + url);

  var request = new XMLHttpRequest();
  request.onload = function() {
    console.log('whereisthe.es response status: ' + request.status);
    if (request.status === 200) {
      try {
        var data = JSON.parse(request.responseText);
        console.log('whereisthe.es data received, stops: ' + (data.Stops ? data.Stops.length : 0));
        var tcsData = transformWhereIsTheEsToTCS(data);
        if (tcsData) {
          dataProcessor.processTCSData(tcsData);
        }
      } catch (e) {
        console.log('Error parsing whereisthe.es response: ' + e);
      }
    } else {
      console.log('whereisthe.es API error: ' + request.status + ' - ' + request.responseText);
    }
  };
  request.onerror = function() {
    console.log('whereisthe.es API request failed');
  };

  request.open('GET', url);
  request.send();
}

module.exports = {
  fetchFromWhereIsTheEs: fetchFromWhereIsTheEs
};
