// NS (Nederlandse Spoorwegen) API Client
// Handles fetching data from NS Reisinformatie API
// API Documentation: https://apiportal.ns.nl/

var state = require('../state');
var dataProcessor = require('../data-processor');
var timeUtils = require('../time-utils');
var helpers = require('./helpers');

// Transform NS API response to TCS format
function transformNSToTCS(data) {
  if (!data || !data.payload || !data.payload.stops || !Array.isArray(data.payload.stops)) {
    console.log('Invalid NS data format');
    return null;
  }

  // Filter to only commercial stops (ORIGIN, STOP, DESTINATION)
  // PASSING stations don't have arrival/departure times in NS API
  var stops = data.payload.stops.filter(function(stop) {
    return stop.status === 'ORIGIN' || stop.status === 'STOP' || stop.status === 'DESTINATION';
  });

  if (stops.length === 0) {
    console.log('No commercial stops found in NS data');
    return null;
  }

  // Transform stops to TCS locations format
  var locations = stops.map(function(stop, idx) {
    var arrival = '';
    var departure = '';
    var arrivalDelay = 0;
    var departureDelay = 0;

    // Process arrivals
    if (stop.arrivals && stop.arrivals.length > 0) {
      var arr = stop.arrivals[0];
      var plannedArrival = helpers.parseNSTime(arr.plannedTime);
      var actualArrival = helpers.parseNSTime(arr.actualTime);

      arrival = actualArrival ? actualArrival.toISOString() : (plannedArrival ? plannedArrival.toISOString() : '');

      if (plannedArrival && actualArrival) {
        arrivalDelay = Math.round((actualArrival - plannedArrival) / 60000);
      }
    }

    // Process departures
    if (stop.departures && stop.departures.length > 0) {
      var dep = stop.departures[0];
      var plannedDeparture = helpers.parseNSTime(dep.plannedTime);
      var actualDeparture = helpers.parseNSTime(dep.actualTime);

      departure = actualDeparture ? actualDeparture.toISOString() : (plannedDeparture ? plannedDeparture.toISOString() : '');

      if (plannedDeparture && actualDeparture) {
        departureDelay = Math.round((actualDeparture - plannedDeparture) / 60000);
      }
    }

    return {
      name: stop.stop && stop.stop.name ? stop.stop.name : 'Unknown',
      arrival: timeUtils.utcToLocalTime(arrival),
      departure: timeUtils.utcToLocalTime(departure),
      arrivalDelay: arrivalDelay,
      departureDelay: departureDelay,
      commercialStop: true
    };
  });

  // Find current station based on real-time data
  var currentStation = null;
  var now = new Date();
  var currentIdx = 0;

  for (var i = 0; i < stops.length; i++) {
    var stop = stops[i];
    var hasDeparted = false;

    // Check if train has departed this station
    if (stop.departures && stop.departures.length > 0) {
      var dep = stop.departures[0];
      var actualDeparture = helpers.parseNSTime(dep.actualTime);
      var plannedDeparture = helpers.parseNSTime(dep.plannedTime);
      var departureTime = actualDeparture || plannedDeparture;

      if (departureTime && departureTime < now) {
        hasDeparted = true;
      }
    }

    if (hasDeparted) {
      continue;
    }

    // This is the current station - train hasn't departed yet
    currentIdx = i;
    var currentStop = stops[i];
    var stopName = currentStop.stop && currentStop.stop.name ? currentStop.stop.name : 'Unknown';

    var arr = currentStop.arrivals && currentStop.arrivals.length > 0 ? currentStop.arrivals[0] : null;
    var dep = currentStop.departures && currentStop.departures.length > 0 ? currentStop.departures[0] : null;

    var plannedArrival = arr ? helpers.parseNSTime(arr.plannedTime) : null;
    var actualArrival = arr ? helpers.parseNSTime(arr.actualTime) : null;
    var plannedDeparture = dep ? helpers.parseNSTime(dep.plannedTime) : null;
    var actualDeparture = dep ? helpers.parseNSTime(dep.actualTime) : null;

    var arrivalTime = actualArrival || plannedArrival;
    var departureTime = actualDeparture || plannedDeparture;

    var arrDelay = 0;
    var depDelay = 0;
    if (plannedArrival && actualArrival) {
      arrDelay = Math.round((actualArrival - plannedArrival) / 60000);
    }
    if (plannedDeparture && actualDeparture) {
      depDelay = Math.round((actualDeparture - plannedDeparture) / 60000);
    }

    currentStation = {
      id: i,
      name: stopName,
      arrival: arrivalTime ? timeUtils.utcToLocalTime(arrivalTime.toISOString()) : '',
      departure: departureTime ? timeUtils.utcToLocalTime(departureTime.toISOString()) : '',
      arrivalDelay: arrDelay,
      departureDelay: depDelay
    };
    break;
  }

  // If no current station found, default to first or last
  if (!currentStation && locations.length > 0) {
    var firstStop = stops[0];
    var firstHasDeparted = false;

    if (firstStop.departures && firstStop.departures.length > 0) {
      var firstDep = firstStop.departures[0];
      var firstActualDep = helpers.parseNSTime(firstDep.actualTime);
      var firstPlannedDep = helpers.parseNSTime(firstDep.plannedTime);
      var firstDepTime = firstActualDep || firstPlannedDep;
      if (firstDepTime && firstDepTime < now) {
        firstHasDeparted = true;
      }
    }

    if (!firstHasDeparted) {
      // Train hasn't started yet
      var firstStopName = firstStop.stop && firstStop.stop.name ? firstStop.stop.name : 'Unknown';
      var firstArr = firstStop.arrivals && firstStop.arrivals.length > 0 ? firstStop.arrivals[0] : null;
      var firstDep = firstStop.departures && firstStop.departures.length > 0 ? firstStop.departures[0] : null;

      var firstArrival = firstArr ? (helpers.parseNSTime(firstArr.actualTime) || helpers.parseNSTime(firstArr.plannedTime)) : null;
      var firstDeparture = firstDep ? (helpers.parseNSTime(firstDep.actualTime) || helpers.parseNSTime(firstDep.plannedTime)) : null;

      currentStation = {
        id: 0,
        name: firstStopName,
        arrival: firstArrival ? timeUtils.utcToLocalTime(firstArrival.toISOString()) : '',
        departure: firstDeparture ? timeUtils.utcToLocalTime(firstDeparture.toISOString()) : '',
        arrivalDelay: 0,
        departureDelay: 0
      };
    } else {
      // Train has completed journey
      var lastIdx = stops.length - 1;
      var lastStop = stops[lastIdx];
      var lastStopName = lastStop.stop && lastStop.stop.name ? lastStop.stop.name : 'Unknown';
      var lastArr = lastStop.arrivals && lastStop.arrivals.length > 0 ? lastStop.arrivals[0] : null;
      var lastDep = lastStop.departures && lastStop.departures.length > 0 ? lastStop.departures[0] : null;

      var lastArrival = lastArr ? (helpers.parseNSTime(lastArr.actualTime) || helpers.parseNSTime(lastArr.plannedTime)) : null;
      var lastDeparture = lastDep ? (helpers.parseNSTime(lastDep.actualTime) || helpers.parseNSTime(lastDep.plannedTime)) : null;

      currentStation = {
        id: lastIdx,
        name: lastStopName,
        arrival: lastArrival ? timeUtils.utcToLocalTime(lastArrival.toISOString()) : '',
        departure: lastDeparture ? timeUtils.utcToLocalTime(lastDeparture.toISOString()) : '',
        arrivalDelay: 0,
        departureDelay: 0
      };
    }
  }

  return {
    locations: locations,
    currentStation: currentStation
  };
}

// Fetch train data from NS API
function fetchFromNS() {
  console.log('Fetching from NS...');

  if (!state.appSettings.trainNumber || !state.appSettings.nsSubscriptionKey) {
    console.log('NS subscription key or train number not configured');
    return;
  }

  // Use configured date or today's date
  var date = state.appSettings.date || helpers.getTodayDate();

  var trainNumber = helpers.stripTrainNumberPrefix(state.appSettings.trainNumber);
  if (!trainNumber) {
    console.log('Invalid train number');
    return;
  }

  // NS Reisinformatie API v2 journey endpoint
  var url = 'https://gateway.apiportal.ns.nl/reisinformatie-api/api/v2/journey?train=' +
            encodeURIComponent(trainNumber) + '&dateTime=' + date + '&omitCrowdForecast=false';

  console.log('NS API URL: ' + url);
  console.log('Train number: ' + trainNumber);

  var request = new XMLHttpRequest();
  request.onload = function() {
    console.log('NS response status: ' + request.status);
    if (request.status === 200) {
      try {
        var data = JSON.parse(request.responseText);
        console.log('NS data received');
        var tcsData = transformNSToTCS(data);
        if (tcsData) {
          dataProcessor.processTCSData(tcsData);
        }
      } catch (e) {
        console.log('Error parsing NS response: ' + e);
      }
    } else {
      console.log('NS API error: ' + request.status + ' - ' + request.responseText);
    }
  };
  request.onerror = function() {
    console.log('NS API request failed');
  };

  request.open('GET', url);
  request.setRequestHeader('Ocp-Apim-Subscription-Key', state.appSettings.nsSubscriptionKey);
  request.send();
}

module.exports = {
  fetchFromNS: fetchFromNS
};
