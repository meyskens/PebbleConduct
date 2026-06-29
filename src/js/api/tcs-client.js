// TCS RailDesk API Client
// Handles fetching data from TCS RailDesk API

var state = require('../state');
var dataProcessor = require('../data-processor');
var timeUtils = require('../time-utils');

// Fetch train data from TCS RailDesk API
function fetchFromTCS() {
  console.log('Fetching from TCS RailDesk...');

  if (!state.appSettings.trainNumber) {
    console.log('Train number not configured');
    return;
  }

  // TCS Raildesk API endpoint - uses train path UUID
  var baseUrl = state.appSettings.tcsUrl;
  // Remove trailing slash from baseUrl to avoid double slashes
  if (baseUrl.endsWith('/')) {
    baseUrl = baseUrl.slice(0, -1);
  }
  var url = baseUrl + '/trainpath/' + state.appSettings.trainNumber;

  console.log('TCS URL: ' + url);
  console.log('Train number (UUID): ' + state.appSettings.trainNumber);
  console.log('Commercial train number: ' + state.appSettings.commercialTrainNumber);

  var request = new XMLHttpRequest();
  request.onload = function() {
    console.log('TCS response status: ' + request.status);
    if (request.status === 200) {
      try {
        var data = JSON.parse(request.responseText);
        console.log('TCS data received, locations: ' + (data.locations ? data.locations.length : 0));
        // Convert UTC times to local times in the API response
        var convertedData = convertTCSTimesToLocal(data);
        dataProcessor.processTCSData(convertedData);
      } catch (e) {
        console.log('Error parsing TCS response: ' + e);
      }
    } else {
      console.log('TCS API error: ' + request.status + ' - ' + request.responseText);
    }
  };
  request.onerror = function() {
    console.log('TCS API request failed');
  };

  request.open('POST', url);
  request.setRequestHeader('Content-Type', 'application/json');
  request.setRequestHeader('Accept', 'application/json');

  // Build payload based on authentication method
  var authMethod = state.appSettings.tcsAuthMethod || 'credentials';
  var payload;

  if (authMethod === 'aspCookie') {
    // ASP.NET cookie authentication
    if (!state.appSettings.tcsAspNetCookieC1 || !state.appSettings.tcsAspNetCookieC2) {
      console.log('TCS ASP.NET cookie not configured');
      return;
    }

    var cookieC1 = state.appSettings.tcsAspNetCookieC1;
    if (typeof cookieC1 === 'object' && cookieC1 !== null && cookieC1.value) {
      cookieC1 = cookieC1.value;
    }
    var cookieC2 = state.appSettings.tcsAspNetCookieC2;
    if (typeof cookieC2 === 'object' && cookieC2 !== null && cookieC2.value) {
      cookieC2 = cookieC2.value;
    }

    payload = JSON.stringify({
      aspNetApplicationCookieC1: cookieC1,
      aspNetApplicationCookieC2: cookieC2
    });
    console.log('Sending TCS request with ASP.NET cookie auth to: ' + url);
  } else {
    // Username & password authentication
    if (!state.appSettings.tcsUsername || !state.appSettings.tcsPassword) {
      console.log('TCS credentials not configured');
      return;
    }

    // Ensure password is a string (not an object from Clay)
    var password = state.appSettings.tcsPassword;
    if (typeof password === 'object' && password !== null && password.value) {
      password = password.value;
      console.log('Extracted password from object wrapper');
    }

    payload = JSON.stringify({
      username: state.appSettings.tcsUsername,
      password: password
    });
    console.log('Sending TCS request to: ' + url);
    console.log('Username: ' + state.appSettings.tcsUsername);
    // Security: do not log password details
  }

  request.send(payload);
}

// Convert TCS data UTC times to local times
function convertTCSTimesToLocal(data) {
  if (!data || !data.locations || !Array.isArray(data.locations)) {
    return data;
  }

  var convertedLocations = data.locations.map(function(loc) {
    return {
      name: loc.name,
      arrival: timeUtils.utcToLocalTime(loc.arrival),
      departure: timeUtils.utcToLocalTime(loc.departure),
      arrivalDelay: loc.arrivalDelay != null ? loc.arrivalDelay : -999,
      departureDelay: loc.departureDelay != null ? loc.departureDelay : -999,
      commercialStop: loc.commercialStop || false
    };
  });

  var convertedCurrentStation = null;
  if (data.currentStation) {
    convertedCurrentStation = {
      name: data.currentStation.name,
      arrival: timeUtils.utcToLocalTime(data.currentStation.arrival),
      departure: timeUtils.utcToLocalTime(data.currentStation.departure),
      arrivalDelay: data.currentStation.arrivalDelay != null ? data.currentStation.arrivalDelay : -999,
      departureDelay: data.currentStation.departureDelay != null ? data.currentStation.departureDelay : -999
    };
  }

  return {
    locations: convertedLocations,
    currentStation: convertedCurrentStation
  };
}

module.exports = {
  fetchFromTCS: fetchFromTCS
};
