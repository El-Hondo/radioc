/**
 * @file favorites.c
 * @brief Management of favorite stations list.
 */
#include <stdio.h>
#include <stdlib.h>
#include <string.h>
#include <unistd.h>
#include "favorites.h"
#include "cJSON.h"

static Station s_favorites[MAX_FAVORITES];
static int s_fav_count = 0;

static const char* get_config_path() {
    static char path[1024];
    const char *home = getenv("HOME");
    if (!home) home = ".";
    snprintf(path, sizeof(path), "%s/.radioc_favorites.json", home);
    return path;
}

int favorites_load() {
    s_fav_count = 0;
    const char *path = get_config_path();
    FILE *f = fopen(path, "r");
    if (!f) return 0;

    fseek(f, 0, SEEK_END);
    long len = ftell(f);
    fseek(f, 0, SEEK_SET);

    char *data = malloc(len + 1);
    fread(data, 1, len, f);
    data[len] = '\0';
    fclose(f);

    cJSON *json = cJSON_Parse(data);
    free(data);

    if (!json) return 0;

    cJSON *item;
    cJSON_ArrayForEach(item, json) {
        if (s_fav_count >= MAX_FAVORITES) break;
        
        cJSON *name = cJSON_GetObjectItemCaseSensitive(item, "name");
        cJSON *url = cJSON_GetObjectItemCaseSensitive(item, "url");
        cJSON *uuid = cJSON_GetObjectItemCaseSensitive(item, "uuid");
        cJSON *country = cJSON_GetObjectItemCaseSensitive(item, "country");
        cJSON *tags = cJSON_GetObjectItemCaseSensitive(item, "tags");

        if (cJSON_IsString(name)) strcpy(s_favorites[s_fav_count].name, name->valuestring);
        if (cJSON_IsString(url)) strcpy(s_favorites[s_fav_count].url, url->valuestring);
        if (cJSON_IsString(uuid)) strcpy(s_favorites[s_fav_count].station_uuid, uuid->valuestring);
        if (cJSON_IsString(country)) strcpy(s_favorites[s_fav_count].country, country->valuestring);
        if (cJSON_IsString(tags)) strcpy(s_favorites[s_fav_count].tags, tags->valuestring);

        s_fav_count++;
    }
    cJSON_Delete(json);
    return s_fav_count;
}

void favorites_save() {
    const char *path = get_config_path();
    cJSON *json = cJSON_CreateArray();

    for (int i = 0; i < s_fav_count; i++) {
        cJSON *item = cJSON_CreateObject();
        cJSON_AddStringToObject(item, "name", s_favorites[i].name);
        cJSON_AddStringToObject(item, "url", s_favorites[i].url);
        cJSON_AddStringToObject(item, "uuid", s_favorites[i].station_uuid);
        cJSON_AddStringToObject(item, "country", s_favorites[i].country);
        cJSON_AddStringToObject(item, "tags", s_favorites[i].tags);
        cJSON_AddItemToArray(json, item);
    }

    char *string = cJSON_Print(json);
    FILE *f = fopen(path, "w");
    if (f) {
        fprintf(f, "%s", string);
        fclose(f);
    }
    
    free(string);
    cJSON_Delete(json);
}

void favorites_add(const Station *s) {
    if (s_fav_count >= MAX_FAVORITES) return;

    // Check for duplicates
    for (int i = 0; i < s_fav_count; i++) {
        if (strcmp(s_favorites[i].station_uuid, s->station_uuid) == 0) {
            return; // Already exists
        }
    }

    s_favorites[s_fav_count] = *s;
    s_fav_count++;
    favorites_save();
}

void favorites_remove(int index) {
    if (index < 0 || index >= s_fav_count) return;
    
    // Shift remaining
    for (int i = index; i < s_fav_count - 1; i++) {
        s_favorites[i] = s_favorites[i+1];
    }
    s_fav_count--;
    favorites_save();
}

Station* favorites_get_list(int *count) {
    *count = s_fav_count;
    return s_favorites;
}
