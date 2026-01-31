#ifndef CONFIG_H
#define CONFIG_H

typedef struct {
    int volume;           // 0-100
    char theme[32];       // e.g. "default", "cyberpunk"
    int last_station_id;  // future use
} AppConfig;

// Load config from disk. Returns 0 on success, -1 on failure (and sets defaults)
int config_load(AppConfig *config);

// Save config to disk.
void config_save(const AppConfig *config);

// Get default config
void config_set_defaults(AppConfig *config);

#endif
