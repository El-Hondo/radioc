#ifndef PLAYER_H
#define PLAYER_H

/**
 * @brief Initialize the libvlc instance.
 * @return 0 on success, -1 on failure.
 */
int player_init();

/**
 * @brief Release all player resources.
 */
void player_cleanup();

/**
 * @brief Play a stream from a URL.
 * @param url The stream URL.
 * @return 0 on success, -1 on error.
 */
int player_play(const char *url);

/**
 * @brief Stop the current playback.
 */
void player_stop();

/**
 * @brief Get current stream metadata (NowPlaying or Title).
 * @return A dynamically allocated string containing the metadata (must be freed by caller), or NULL.
 */
char* player_get_metadata();

/**
 * @brief Adjust volume by a relative amount.
 * @param adjustment The amount to change volume by (e.g., +5 or -5).
 * @return The new volume level (0-100).
 */
int player_set_volume(int adjustment);

/**
 * @brief Get the current volume.
 * @return Volume level (0-100).
 */
int player_get_volume();

#endif
