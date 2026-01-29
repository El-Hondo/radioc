#ifndef API_H
#define API_H

#define MAX_STATIONS 100

typedef struct {
    char name[256];
    char url[512];
    char country[128];
    char tags[256];
    char station_uuid[64];
} Station;

// Searches for stations by name/tag.
// Returns the number of stations found, or -1 on error.
// Fills the 'results' array up to 'max_results'.
int api_search_stations(const char *query, Station *results, int max_results);

// Searches for stations by country code (e.g., 'US', 'DE')
int api_search_by_country(const char *country_code, Station *results, int max_results);

#endif
