#ifndef PLAYER_H
#define PLAYER_H

// Initialize libvlc
int player_init();

// Cleanup resources
void player_cleanup();

// Play a stream URL
// Returns 0 on success, -1 on error
int player_play(const char *url);

// Stop current playback
void player_stop();

// Get current stream metadata (NowPlaying or Title)
// Returns a dynamically allocated string that must be freed by the caller,
// or NULL if no metadata is available.
char* player_get_metadata();

// Adjust volume by relative amount (+/-). Returns new volume (0-100).
int player_set_volume(int adjustment);

// Get current volume
int player_get_volume();

#endif
