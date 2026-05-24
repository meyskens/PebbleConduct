// Time utilities for PebbleConduct
// Handles time conversions and formatting

// Convert UTC time string to local time string (HH:MM format)
// Handles both ISO timestamps (2026-05-19T16:13:00Z) and simple HH:MM format
function utcToLocalTime(utcTimestamp) {
  if (!utcTimestamp) return '';
  
  try {
    var date;
    
    // Check if it's already in HH:MM format
    if (/^\d{1,2}:\d{2}$/.test(utcTimestamp)) {
      // Parse as UTC time today: create date with time in UTC
      var parts = utcTimestamp.split(':');
      var hours = parseInt(parts[0], 10);
      var minutes = parseInt(parts[1], 10);
      
      date = new Date();
      // Set to UTC time
      date.setUTCHours(hours, minutes, 0, 0);
    } else {
      // Try parsing as ISO timestamp
      date = new Date(utcTimestamp);
      if (isNaN(date.getTime())) return utcTimestamp;
    }
    
    // Convert to local time
    var localHours = date.getHours();
    var localMinutes = date.getMinutes();
    
    // Pad with leading zeros
    var hoursStr = localHours < 10 ? '0' + localHours : localHours.toString();
    var minutesStr = localMinutes < 10 ? '0' + localMinutes : localMinutes.toString();
    
    return hoursStr + ':' + minutesStr;
  } catch (e) {
    console.log('Error converting UTC time: ' + e);
    return utcTimestamp;
  }
}

module.exports = {
  utcToLocalTime: utcToLocalTime
};
