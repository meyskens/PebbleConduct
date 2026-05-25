// TCS RailDesk API Fake Server
// Simulates the TCS RailDesk API for testing purposes
package main

import (
	"encoding/json"
	"flag"
	"fmt"
	"log"
	"net/http"
	"os"
	"strconv"
	"strings"
	"time"
)

// Location represents a single stop in the timetable
type Location struct {
	Name           string `json:"name"`
	Arrival        string `json:"arrival"`
	Departure      string `json:"departure"`
	ArrivalDelay   *int   `json:"arrivalDelay"`
	DepartureDelay *int   `json:"departureDelay"`
	CommercialStop bool   `json:"commercialStop"`
}

// TimetableResponse represents the API response structure
type TimetableResponse struct {
	CurrentStation CurrentStation `json:"currentStation"`
	Locations      []Location     `json:"locations"`
}

// CurrentStation represents the current position of the train
type CurrentStation struct {
	ID             int    `json:"id"`
	Name           string `json:"name"`
	Arrival        string `json:"arrival"`
	Departure      string `json:"departure"`
	ArrivalDelay   *int   `json:"arrivalDelay"`
	DepartureDelay *int   `json:"departureDelay"`
}

// Credentials represents the login request body
type Credentials struct {
	Username string `json:"username"`
	Password string `json:"password"`
}

var (
	startTimeOverride string
	timetableData     TimetableResponse
	baseStartTime     time.Time
	timeOffset        time.Duration
)

func main() {
	var port int
	var timetableFile string

	flag.IntVar(&port, "port", 8080, "Port to run the server on")
	flag.StringVar(&timetableFile, "timetable", "timetable.json", "Path to timetable JSON file")
	flag.StringVar(&startTimeOverride, "start-time", "", "Override first stop time (HH:MM format)")
	flag.Parse()

	// Load timetable data
	if err := loadTimetable(timetableFile); err != nil {
		log.Fatalf("Failed to load timetable: %v", err)
	}

	// Calculate time offset if start-time is provided
	if startTimeOverride != "" {
		calculateTimeOffset()
	}

	mux := http.NewServeMux()

	// TCS RailDesk API endpoints
	mux.HandleFunc("POST /trainpath/{uuid}", handleTrainPath)

	fmt.Printf("TCS RailDesk Fake Server starting on port %d...\n", port)
	if startTimeOverride != "" {
		fmt.Printf("Using simulated start time: %s (offset: %s)\n", startTimeOverride, timeOffset)
	} else {
		fmt.Println("Using actual wall clock time")
	}
	fmt.Printf("Loaded %d locations from timetable\n", len(timetableData.Locations))

	log.Fatal(http.ListenAndServe(fmt.Sprintf(":%d", port), mux))
}

func loadTimetable(filename string) error {
	data, err := os.ReadFile(filename)
	if err != nil {
		return fmt.Errorf("reading timetable file: %w", err)
	}

	var rawData struct {
		Locations []Location `json:"locations"`
	}
	if err := json.Unmarshal(data, &rawData); err != nil {
		return fmt.Errorf("parsing timetable JSON: %w", err)
	}

	timetableData.Locations = rawData.Locations
	return nil
}

func calculateTimeOffset() {
	if len(timetableData.Locations) == 0 {
		return
	}

	// Find the first location with a departure time
	var firstDeparture string
	for _, loc := range timetableData.Locations {
		if loc.Departure != "" {
			firstDeparture = loc.Departure
			break
		}
	}

	if firstDeparture == "" {
		log.Println("Warning: No departure time found in timetable")
		return
	}

	// Parse the original first departure time
	originalTime, err := time.Parse("15:04", firstDeparture)
	if err != nil {
		log.Printf("Warning: Could not parse first departure time '%s': %v", firstDeparture, err)
		return
	}

	// Parse the override start time
	overrideTime, err := time.Parse("15:04", startTimeOverride)
	if err != nil {
		log.Printf("Warning: Could not parse --start-time '%s': %v", startTimeOverride, err)
		return
	}

	// Calculate offset: how much to shift the timetable
	// If original first departure is 16:15 and we want 16:20, offset is +5m (shift timetable forward by 5 min)
	timeOffset = overrideTime.Sub(originalTime)
	baseStartTime = time.Date(2000, 1, 1, overrideTime.Hour(), overrideTime.Minute(), 0, 0, time.UTC)

	log.Printf("Time offset calculated: %v (original first departure: %s, new: %s)",
		timeOffset, firstDeparture, startTimeOverride)
}

func handleTrainPath(w http.ResponseWriter, r *http.Request) {
	uuid := r.PathValue("uuid")

	log.Printf("Received request for trainpath: %s from %s", uuid, r.RemoteAddr)

	// Parse credentials (optional - we accept any credentials)
	var creds Credentials
	if err := json.NewDecoder(r.Body).Decode(&creds); err != nil {
		log.Printf("Warning: Could not parse credentials: %v", err)
	}

	log.Printf("Login attempt - Username: %s", creds.Username)

	// Generate response based on current time
	response := generateResponse()

	w.Header().Set("Content-Type", "application/json")
	w.WriteHeader(http.StatusOK)
	json.NewEncoder(w).Encode(response)
}

func generateResponse() TimetableResponse {
	now := time.Now()

	// Apply time offset to current time for comparison
	// If offset is -1h57m (timetable shifted earlier), we add 1h57m to current time
	// to find where we are in the original timetable
	if startTimeOverride != "" {
		now = now.Add(-timeOffset)
	}

	currentTimeStr := now.Format("15:04")
	currentMinutes := timeToMinutes(currentTimeStr)

	// Find current station based on time
	currentStation := findCurrentStation(currentMinutes)

	// Build locations with adjusted times if needed
	locations := make([]Location, len(timetableData.Locations))
	for i, loc := range timetableData.Locations {
		locations[i] = Location{
			Name:           loc.Name,
			Arrival:        adjustTime(loc.Arrival),
			Departure:      adjustTime(loc.Departure),
			ArrivalDelay:   loc.ArrivalDelay,
			DepartureDelay: loc.DepartureDelay,
			CommercialStop: loc.CommercialStop,
		}
	}

	return TimetableResponse{
		CurrentStation: currentStation,
		Locations:      locations,
	}
}

func adjustTime(t string) string {
	if t == "" {
		return ""
	}

	if startTimeOverride == "" {
		return t
	}

	// Parse the original time
	originalMinutes := timeToMinutes(t)
	if originalMinutes < 0 {
		return t
	}

	// Apply offset (add because timeOffset is already the shift amount)
	adjustedMinutes := originalMinutes + int(timeOffset.Minutes())

	// Handle day wrap
	for adjustedMinutes < 0 {
		adjustedMinutes += 24 * 60
	}
	adjustedMinutes = adjustedMinutes % (24 * 60)

	return minutesToTime(adjustedMinutes)
}

func findCurrentStation(currentMinutes int) CurrentStation {
	if len(timetableData.Locations) == 0 {
		return CurrentStation{ID: -1, Name: "Unknown"}
	}

	// Find the station we're currently at or just passed
	// Consider both arrival and departure times to account for dwell time at stations
	var currentIdx = -1
	var currentLoc Location

	for i, loc := range timetableData.Locations {
		// Use original times for comparison (current time is already adjusted)
		depMinutes := timeToMinutes(loc.Departure)
		arrMinutes := timeToMinutes(loc.Arrival)

		// Determine the "end time" for this station (when we leave it)
		// Use departure time if available, otherwise arrival time
		endTime := depMinutes
		if endTime < 0 {
			endTime = arrMinutes
		}

		// Determine the "start time" for this station (when we arrive at it)
		// Use arrival time if available, otherwise departure time
		startTime := arrMinutes
		if startTime < 0 {
			startTime = depMinutes
		}

		if endTime < 0 {
			continue
		}

		// Check if we're currently at this station or have passed it
		// We're at this station if current time is between arrival and departure
		// We've passed this station if current time >= departure time
		if currentMinutes >= endTime {
			// We've passed this station
			currentIdx = i
			currentLoc = loc
		} else if startTime >= 0 && currentMinutes >= startTime && currentMinutes < endTime {
			// We're currently at this station (between arrival and departure)
			currentIdx = i
			currentLoc = loc
			break
		} else {
			// We haven't reached this station yet
			break
		}
	}

	// If we haven't reached any station yet, return the first one
	if currentIdx < 0 {
		currentIdx = 0
		currentLoc = timetableData.Locations[0]
	}

	return CurrentStation{
		ID:             currentIdx,
		Name:           currentLoc.Name,
		Arrival:        adjustTime(currentLoc.Arrival),
		Departure:      adjustTime(currentLoc.Departure),
		ArrivalDelay:   currentLoc.ArrivalDelay,
		DepartureDelay: currentLoc.DepartureDelay,
	}
}

func timeToMinutes(t string) int {
	if t == "" {
		return -1
	}

	parts := strings.Split(t, ":")
	if len(parts) != 2 {
		return -1
	}

	hours, err1 := strconv.Atoi(parts[0])
	minutes, err2 := strconv.Atoi(parts[1])

	if err1 != nil || err2 != nil {
		return -1
	}

	return hours*60 + minutes
}

func minutesToTime(minutes int) string {
	hours := minutes / 60
	mins := minutes % 60
	return fmt.Sprintf("%02d:%02d", hours, mins)
}
