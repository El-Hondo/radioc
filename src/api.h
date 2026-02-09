#ifndef API_H
#define API_H

#define MAX_STATIONS 250
#define RA_VERSION "0.2"

typedef struct {
    char name[256];
    char url[512];
    char country[128];
    char tags[256];
    char station_uuid[64];
    int votes;
} Station;

/**
 * @brief Searches for stations by name or tag.
 * @param query The search string.
 * @param results Array to store found stations.
 * @param max_results Maximum number of results to return (capacity of results array).
 * @return Number of stations found, or -1 on error.
 */
int api_search_stations(const char *query, Station *results, int max_results);

/**
 * @brief Searches for stations by country code.
 * @param country_code Two-letter country code (e.g., "US", "DE").
 * @param results Array to store found stations.
 * @param max_results Maximum number of results to return.
 * @return Number of stations found, or -1 on error.
 */
int api_search_by_country(const char *country_code, Station *results, int max_results);

/**
 * @brief Searches for stations by tag.
 * @param tag The tag to search for (e.g., "jazz", "news").
 * @param results Array to store found stations.
 * @param max_results Maximum number of results to return.
 * @return Number of stations found, or -1 on error.
 */
int api_search_by_tag(const char *tag, Station *results, int max_results);

#endif
