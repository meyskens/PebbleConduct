// API Client Helpers
// Shared utility functions for API clients

/**
 * Strip non-digit prefix from train number (e.g., "ES453" -> "453")
 * @param {string} trainNumber - The train number with optional prefix
 * @returns {string} - The numeric portion of the train number
 */
function stripTrainNumberPrefix(trainNumber) {
  if (!trainNumber) return '';
  var match = trainNumber.match(/[0-9].*/);
  return match ? match[0] : '';
}

/**
 * Check if a timestamp is valid (not empty, null, or the "0001-01-01" null value)
 * @param {string} timestamp - The timestamp to validate
 * @returns {boolean} - True if the timestamp is valid
 */
function isValidTime(timestamp) {
  return timestamp && timestamp !== '' && !timestamp.startsWith('0001-01-01');
}

/**
 * Calculate delay in minutes between scheduled and actual times
 * Can be negative for early arrivals/departures
 * @param {string} scheduled - The scheduled time (ISO string)
 * @param {string} actual - The actual time (ISO string)
 * @returns {number} - Delay in minutes (can be negative)
 */
function calculateDelay(scheduled, actual) {
  if (!isValidTime(scheduled) || !isValidTime(actual)) {
    return 0;
  }
  var scheduledDate = new Date(scheduled);
  var actualDate = new Date(actual);
  var diffMs = actualDate - scheduledDate;
  return Math.round(diffMs / 60000);
}

/**
 * Parse ISO-8601 timestamp to Date object
 * @param {string} timestamp - ISO-8601 timestamp
 * @returns {Date|null} - Date object or null if invalid
 */
function parseISOTime(timestamp) {
  if (!timestamp) return null;
  try {
    return new Date(timestamp);
  } catch (e) {
    return null;
  }
}

/**
 * Parse NS timestamp to Date object
 * NS uses RFC3339 without colon in timezone offset (e.g., "2026-04-29T06:06:00+0200")
 * @param {string} timestamp - NS timestamp format
 * @returns {Date|null} - Date object or null if invalid
 */
function parseNSTime(timestamp) {
  if (!timestamp) return null;
  try {
    // NS format: 2026-04-29T06:06:00+0200
    // Convert to standard ISO format by inserting colon in timezone
    var match = timestamp.match(/^(\d{4}-\d{2}-\d{2}T\d{2}:\d{2}:\d{2})([+-]\d{2})(\d{2})$/);
    if (match) {
      var isoTimestamp = match[1] + match[2] + ':' + match[3];
      return new Date(isoTimestamp);
    }
    // Fallback to direct parsing
    return new Date(timestamp);
  } catch (e) {
    return null;
  }
}

/**
 * Format Date to local time string (HH:MM)
 * @param {Date} date - Date object
 * @returns {string} - Formatted time string
 */
function formatTimeLocal(date) {
  if (!date) return '';
  var hours = String(date.getHours()).padStart(2, '0');
  var minutes = String(date.getMinutes()).padStart(2, '0');
  return hours + ':' + minutes;
}

/**
 * Get today's date in YYYY-MM-DD format
 * @returns {string} - Today's date as YYYY-MM-DD
 */
function getTodayDate() {
  var today = new Date();
  var year = today.getFullYear();
  var month = String(today.getMonth() + 1).padStart(2, '0');
  var day = String(today.getDate()).padStart(2, '0');
  return year + '-' + month + '-' + day;
}

module.exports = {
  stripTrainNumberPrefix: stripTrainNumberPrefix,
  isValidTime: isValidTime,
  calculateDelay: calculateDelay,
  parseISOTime: parseISOTime,
  parseNSTime: parseNSTime,
  formatTimeLocal: formatTimeLocal,
  getTodayDate: getTodayDate
};
