#include <stdio.h>
#include <stdlib.h>
#include <string.h>
#include "config.h"
#include "cJSON.h"

static const char* get_config_path() {
    static char path[1024];
    const char *home = getenv("HOME");
    if (!home) home = ".";
    // We'll store it alongside favorites
    snprintf(path, sizeof(path), "%s/.viberadio_config.json", home);
    return path;
}

void config_set_defaults(AppConfig *config) {
    if (!config) return;
    config->volume = 50;
    strncpy(config->theme, "default", sizeof(config->theme) - 1);
    config->theme[sizeof(config->theme) - 1] = '\0';
    config->last_station_id = 0;
}

int config_load(AppConfig *config) {
    if (!config) return -1;
    config_set_defaults(config);

    const char *path = get_config_path();
    FILE *f = fopen(path, "r");
    if (!f) return -1;

    fseek(f, 0, SEEK_END);
    long len = ftell(f);
    fseek(f, 0, SEEK_SET);
    
    if (len <= 0) { fclose(f); return -1; }

    char *data = malloc(len + 1);
    if (!data) { fclose(f); return -1; }
    
    fread(data, 1, len, f);
    data[len] = '\0';
    fclose(f);

    cJSON *json = cJSON_Parse(data);
    free(data);

    if (!json) return -1;

    cJSON *volume = cJSON_GetObjectItemCaseSensitive(json, "volume");
    if (cJSON_IsNumber(volume)) {
        config->volume = volume->valueint;
    }

    cJSON *theme = cJSON_GetObjectItemCaseSensitive(json, "theme");
    if (cJSON_IsString(theme)) {
        strncpy(config->theme, theme->valuestring, sizeof(config->theme) - 1);
        config->theme[sizeof(config->theme) - 1] = '\0';
    }

    cJSON_Delete(json);
    return 0;
}

void config_save(const AppConfig *config) {
    if (!config) return;

    cJSON *json = cJSON_CreateObject();
    cJSON_AddNumberToObject(json, "volume", config->volume);
    cJSON_AddStringToObject(json, "theme", config->theme);

    char *string = cJSON_Print(json);
    cJSON_Delete(json);

    const char *path = get_config_path();
    FILE *f = fopen(path, "w");
    if (f) {
        fprintf(f, "%s", string);
        fclose(f);
    }
    
    free(string);
}
