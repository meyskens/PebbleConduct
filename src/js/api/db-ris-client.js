// DB RIS API Client
// Handles fetching data from Deutsche Bahn RIS API

var state = require('../state');
var dataProcessor = require('../data-processor');
var timeUtils = require('../time-utils');
var helpers = require('./helpers');

// Group DB RIS events into stops (merging arrival and departure per station)
function groupDBRISEvents(events) {
  var stopsByEva = {};
  var order = [];

  events.forEach(function(ev) {
    var eva = ev.stopPlace && ev.stopPlace.evaNumber;
    if (!eva) return;

    if (!stopsByEva[eva]) {
      stopsByEva[eva] = {
        name: ev.stopPlace.name || 'Unknown',
        evaNumber: eva,
        arrival: null,
        departure: null,
        arrivalRealtime: null,
        departureRealtime: null,
        platform: '',
        platformRealtime: '',
        cancelled: false
      };
      order.push(eva);
    }

    var stop = stopsByEva[eva];
    var scheduled = helpers.parseISOTime(ev.timeSchedule);
    var realtime = helpers.parseISOTime(ev.time);

    if (ev.type === 'ARRIVAL') {
      stop.arrival = scheduled;
      stop.arrivalRealtime = realtime;
      if (!stop.platform && ev.platformSchedule) {
        stop.platform = ev.platformSchedule;
      }
      if (!stop.platformRealtime && ev.platform) {
        stop.platformRealtime = ev.platform;
      }
    } else if (ev.type === 'DEPARTURE') {
      stop.departure = scheduled;
      stop.departureRealtime = realtime;
      if (ev.platformSchedule) {
        stop.platform = ev.platformSchedule;
      } else if (!stop.platform && ev.platform) {
        stop.platform = ev.platform;
      }
      if (ev.platform) {
        stop.platformRealtime = ev.platform;
      }
    }

    if (ev.cancelled) {
      stop.cancelled = true;
    }
  });

  return order.map(function(eva) {
    return stopsByEva[eva];
  });
}

// Transform DB RIS stops to TCS format
function transformDBRISToTCS(stops) {
  if (!stops || !Array.isArray(stops) || stops.length === 0) {
    console.log('Invalid DB RIS stops data');
    return null;
  }

  // Transform stops to TCS locations format
  var locations = stops.map(function(stop, idx) {
    var arrival = stop.arrivalRealtime || stop.arrival;
    var departure = stop.departureRealtime || stop.departure;

    // Calculate delays in minutes
    var arrivalDelay = 0;
    var departureDelay = 0;
    if (stop.arrival && stop.arrivalRealtime) {
      arrivalDelay = Math.round((stop.arrivalRealtime - stop.arrival) / 60000);
    }
    if (stop.departure && stop.departureRealtime) {
      departureDelay = Math.round((stop.departureRealtime - stop.departure) / 60000);
    }

    // Convert to local time strings before returning
    return {
      name: stop.name,
      arrival: arrival ? timeUtils.utcToLocalTime(arrival.toISOString()) : '',
      departure: departure ? timeUtils.utcToLocalTime(departure.toISOString()) : '',
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
    var departureTime = stop.departureRealtime || stop.departure;

    // Check if train has departed this station
    if (departureTime && departureTime < now) {
      continue;
    }

    // This is the current station - train hasn't departed yet
    var arrival = stop.arrivalRealtime || stop.arrival;
    var departure = stop.departureRealtime || stop.departure;
    var arrivalDelay = 0;
    var departureDelay = 0;
    if (stop.arrival && stop.arrivalRealtime) {
      arrivalDelay = Math.round((stop.arrivalRealtime - stop.arrival) / 60000);
    }
    if (stop.departure && stop.departureRealtime) {
      departureDelay = Math.round((stop.departureRealtime - stop.departure) / 60000);
    }

    currentIdx = i;
    // Convert to local time strings before returning
    currentStation = {
      id: i,
      name: stop.name,
      arrival: arrival ? timeUtils.utcToLocalTime(arrival.toISOString()) : '',
      departure: departure ? timeUtils.utcToLocalTime(departure.toISOString()) : '',
      arrivalDelay: arrivalDelay,
      departureDelay: departureDelay
    };
    break;
  }

  // If no current station found, default to first or last
  if (!currentStation && locations.length > 0) {
    var firstStop = stops[0];
    var hasStarted = firstStop.departureRealtime && firstStop.departureRealtime < now;

    if (!hasStarted) {
      // Train hasn't started yet
      var arrival = firstStop.arrivalRealtime || firstStop.arrival;
      var departure = firstStop.departureRealtime || firstStop.departure;
      // Convert to local time strings before returning
      currentStation = {
        id: 0,
        name: firstStop.name,
        arrival: arrival ? timeUtils.utcToLocalTime(arrival.toISOString()) : '',
        departure: departure ? timeUtils.utcToLocalTime(departure.toISOString()) : '',
        arrivalDelay: 0,
        departureDelay: 0
      };
    } else {
      // Train has completed journey
      var lastIdx = stops.length - 1;
      var lastStop = stops[lastIdx];
      var lastArrival = lastStop.arrivalRealtime || lastStop.arrival;
      var lastDeparture = lastStop.departureRealtime || lastStop.departure;
      // Convert to local time strings before returning
      currentStation = {
        id: lastIdx,
        name: lastStop.name,
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

// Fetch journey events for a specific journey ID
function fetchDBRISJourneyEvents(journeyID, keyId, keySecret) {
  return new Promise(function(resolve, reject) {
    var url = 'https://apis.deutschebahn.com/db/apis/ris-journeys/v2/' + encodeURIComponent(journeyID);
    console.log('Fetching DB RIS journey events from: ' + url);

    var request = new XMLHttpRequest();
    request.onload = function() {
      console.log('DB RIS events response status: ' + request.status);
      if (request.status === 200) {
        try {
          var data = JSON.parse(request.responseText);
          resolve(data.events || []);
        } catch (e) {
          reject('Error parsing DB RIS events response: ' + e);
        }
      } else {
        reject('DB RIS events API error: ' + request.status + ' - ' + request.responseText);
      }
    };
    request.onerror = function() {
      reject('DB RIS events API request failed');
    };

    request.open('GET', url);
    request.setRequestHeader('DB-Client-Id', keyId);
    request.setRequestHeader('DB-Api-Key', keySecret);
    request.send();
  });
}

// Fetch train data from DB RIS API
function fetchFromDBRIS() {
  console.log('Fetching from DB RIS...');

  if (!state.appSettings.trainNumber || !state.appSettings.dbClientId || !state.appSettings.dbApiKey) {
    console.log('DB RIS credentials or train number not configured');
    return;
  }

  // Use configured date or today's date
  var date = state.appSettings.date || helpers.getTodayDate();

  var trainNumber = helpers.stripTrainNumberPrefix(state.appSettings.trainNumber);
  if (!trainNumber) {
    console.log('Invalid train number');
    return;
  }

  // DB RIS API v2 find endpoint
  var transportTypes = 'HIGH_SPEED_TRAIN,INTERCITY_TRAIN,INTER_REGIONAL_TRAIN,REGIONAL_TRAIN,CITY_TRAIN';
  var url = 'https://apis.deutschebahn.com/db/apis/ris-journeys/v2/find?journeyNumber=' +
            encodeURIComponent(trainNumber) + '&date=' + date + '&transportTypes=' + transportTypes;

  console.log('DB RIS find URL: ' + url);
  console.log('Train number: ' + trainNumber);

  var request = new XMLHttpRequest();
  request.onload = function() {
    console.log('DB RIS find response status: ' + request.status);
    if (request.status === 200) {
      try {
        var data = JSON.parse(request.responseText);
        var journeys = data.journeys || [];
        console.log('DB RIS found ' + journeys.length + ' journeys');

        if (journeys.length === 0) {
          console.log('No journey found for train ' + state.appSettings.trainNumber);
          return;
        }

        // Parse train number input (e.g., "IC3420" -> category: "IC", number: "3420")
        var trainNumberInput = state.appSettings.trainNumber || '';
        var categoryMatch = trainNumberInput.match(/^([A-Za-z]+)?([0-9]+)$/);
        var inputCategory = categoryMatch && categoryMatch[1] ? categoryMatch[1].toUpperCase() : '';
        var inputNumber = categoryMatch && categoryMatch[2] ? categoryMatch[2] : trainNumberInput;

        console.log('Looking for train: category=' + inputCategory + ', number=' + inputNumber);

        // Find journey matching category and number, or use first result
        var selectedJourney = journeys[0];

        for (var i = 0; i < journeys.length; i++) {
          var j = journeys[i];
          var transport = j.info && j.info.transportAtStart;
          var journeyCategory = transport && transport.category ? transport.category.toUpperCase() : '';
          var journeyNumber = j.info && j.info.headerJourneyNumber ? String(j.info.headerJourneyNumber) : '';

          // Match by number (required) and category (if provided)
          var numberMatches = journeyNumber === inputNumber;
          var categoryMatches = !inputCategory || journeyCategory === inputCategory;

          if (numberMatches && categoryMatches) {
            selectedJourney = j;
            console.log('Found match: ' + journeyCategory + ' ' + journeyNumber + ' to ' + j.info.destination.name);
            break;
          }
        }

        var journeyID = selectedJourney.journeyID;
        console.log('Using journey ID: ' + journeyID);

        // Fetch events for this journey
        fetchDBRISJourneyEvents(journeyID, state.appSettings.dbClientId, state.appSettings.dbApiKey)
          .then(function(events) {
            console.log('DB RIS events received: ' + events.length);
            var stops = groupDBRISEvents(events);
            console.log('Grouped into ' + stops.length + ' stops');
            var tcsData = transformDBRISToTCS(stops);
            if (tcsData) {
              dataProcessor.processTCSData(tcsData);
            }
          })
          .catch(function(error) {
            console.log('Error fetching journey events: ' + error);
          });

      } catch (e) {
        console.log('Error parsing DB RIS find response: ' + e);
      }
    } else {
      console.log('DB RIS find API error: ' + request.status + ' - ' + request.responseText);
    }
  };
  request.onerror = function() {
    console.log('DB RIS find API request failed');
  };

  request.open('GET', url);
  request.setRequestHeader('DB-Client-Id', state.appSettings.dbClientId);
  request.setRequestHeader('DB-Api-Key', state.appSettings.dbApiKey);
  request.send();
}

module.exports = {
  fetchFromDBRIS: fetchFromDBRIS
};
