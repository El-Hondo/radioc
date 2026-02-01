/**
 * @file api.c
 * @brief API interaction for searching stations.
 */
#include <stdio.h>
#include <stdlib.h>
#include <string.h>
#include <curl/curl.h>
#include "cJSON.h"
#include "api.h"

// Helper struct for curl memory
struct MemoryStruct {
    char *memory;
    size_t size;
};

static size_t WriteMemoryCallback(void *contents, size_t size, size_t nmemb, void *userp) {
    size_t realsize = size * nmemb;
    struct MemoryStruct *mem = (struct MemoryStruct *)userp;

    char *ptr = realloc(mem->memory, mem->size + realsize + 1);
    if (!ptr) {
        printf("Not enough memory (realloc returned NULL)\n");
        return 0;
    }

    mem->memory = ptr;
    memcpy(&(mem->memory[mem->size]), contents, realsize);
    mem->size += realsize;
    mem->memory[mem->size] = 0;

    return realsize;
}

static int api_search_generic(const char *param, const char *query, Station *results, int max_results) {
    CURL *curl_handle;
    CURLcode res;
    struct MemoryStruct chunk;

    chunk.memory = malloc(1);
    chunk.size = 0;

    curl_global_init(CURL_GLOBAL_ALL);
    curl_handle = curl_easy_init();

    if (!curl_handle) {
        free(chunk.memory);
        return -1;
    }

    // Construct URL
    char url[1024];
    char *encoded_query = curl_easy_escape(curl_handle, query, 0);
    // Use the provided param name (e.g. "name" or "countrycode")
    // Added order=votes and reverse=true to get highest voted stations first
    snprintf(url, sizeof(url), "http://de1.api.radio-browser.info/json/stations/search?%s=%s&limit=%d&order=votes&reverse=true", param, encoded_query, max_results);
    curl_free(encoded_query);

    curl_easy_setopt(curl_handle, CURLOPT_URL, url);
    curl_easy_setopt(curl_handle, CURLOPT_WRITEFUNCTION, WriteMemoryCallback);
    curl_easy_setopt(curl_handle, CURLOPT_WRITEDATA, (void *)&chunk);
    curl_easy_setopt(curl_handle, CURLOPT_USERAGENT, "radioc/1.0");

    res = curl_easy_perform(curl_handle);

    if (res != CURLE_OK) {
        fprintf(stderr, "curl_easy_perform() failed: %s\n", curl_easy_strerror(res));
        free(chunk.memory);
        curl_easy_cleanup(curl_handle);
        return -1;
    }

    // Parse JSON
    cJSON *json = cJSON_Parse(chunk.memory);
    int count = 0;

    if (json == NULL) {
        const char *error_ptr = cJSON_GetErrorPtr();
        if (error_ptr != NULL) {
            fprintf(stderr, "Error before: %s\n", error_ptr);
        }
        res = -1;
    } else {
        cJSON *item = NULL;
        cJSON_ArrayForEach(item, json) {
            if (count >= max_results) break;

            cJSON *name = cJSON_GetObjectItemCaseSensitive(item, "name");
            cJSON *url_resolved = cJSON_GetObjectItemCaseSensitive(item, "url_resolved"); 
            // fallback to 'url' if resolved is empty/null, but API usually sends url_resolved
            if (!cJSON_IsString(url_resolved) || (url_resolved->valuestring == NULL)) {
                 url_resolved = cJSON_GetObjectItemCaseSensitive(item, "url");
            }
            
            cJSON *country = cJSON_GetObjectItemCaseSensitive(item, "country");
            cJSON *tags = cJSON_GetObjectItemCaseSensitive(item, "tags");
            cJSON *votes = cJSON_GetObjectItemCaseSensitive(item, "votes");
            cJSON *uuid = cJSON_GetObjectItemCaseSensitive(item, "stationuuid");

            if (cJSON_IsString(name) && (name->valuestring != NULL)) {
                snprintf(results[count].name, sizeof(results[count].name), "%s", name->valuestring);
            } else {
                 strcpy(results[count].name, "Unknown");
            }

            if (cJSON_IsString(url_resolved) && (url_resolved->valuestring != NULL)) {
                 snprintf(results[count].url, sizeof(results[count].url), "%s", url_resolved->valuestring);
            } else {
                 results[count].url[0] = '\0';
            }

            if (cJSON_IsString(country) && (country->valuestring != NULL)) {
                 snprintf(results[count].country, sizeof(results[count].country), "%s", country->valuestring);
            }
            
            if (cJSON_IsString(tags) && (tags->valuestring != NULL)) {
                 snprintf(results[count].tags, sizeof(results[count].tags), "%s", tags->valuestring);
            }

            if (cJSON_IsString(uuid) && (uuid->valuestring != NULL)) {
                 snprintf(results[count].station_uuid, sizeof(results[count].station_uuid), "%s", uuid->valuestring);
            }

            if (cJSON_IsNumber(votes)) {
                results[count].votes = votes->valueint;
            } else {
                results[count].votes = 0;
            }

            count++;
        }
        cJSON_Delete(json);
        res = count;
    }

    curl_easy_cleanup(curl_handle);
    free(chunk.memory);
    curl_global_cleanup();

    return res;
}

int api_search_stations(const char *query, Station *results, int max_results) {
    return api_search_generic("name", query, results, max_results);
}

int api_search_by_country(const char *country_code, Station *results, int max_results) {
    return api_search_generic("countrycode", country_code, results, max_results);
}

int api_search_by_tag(const char *tag, Station *results, int max_results) {
    return api_search_generic("tag", tag, results, max_results);
}
