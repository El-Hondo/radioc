#ifndef CONFIG_H
#define CONFIG_H

typedef struct {
    int volume;           // 0-100
    char theme[32];       // e.g. "default", "cyberpunk"
    int last_station_id;  // future use
} AppConfig;

/**
 * @brief Load configuration from disk.
 * @param config Pointer to AppConfig struct to fill.
 * @return 0 on success, -1 on failure (defaults will be set on failure).
 */
int config_load(AppConfig *config);

/**
 * @brief Save configuration to disk.
 * @param config Pointer to the AppConfig struct to save.
 */
void config_save(const AppConfig *config);

/**
 * @brief Set default values for the configuration.
 * @param config Pointer to the AppConfig struct to initialize.
 */
void config_set_defaults(AppConfig *config);

#endif
