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

#endif
