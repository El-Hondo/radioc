#include <stdio.h>
#include <stdlib.h>
#include <vlc/vlc.h>
#include "player.h"

static libvlc_instance_t * inst;
static libvlc_media_player_t *mp;

int player_init() {
    const char *vlc_args[] = { "--quiet", "--no-video" };
    inst = libvlc_new(2, vlc_args);
    if (!inst) {
        fprintf(stderr, "Failed to create libvlc instance\n");
        return -1;
    }
    mp = NULL;
    return 0;
}

void player_cleanup() {
    if (mp) {
        libvlc_media_player_stop(mp);
        libvlc_media_player_release(mp);
    }
    if (inst) {
        libvlc_release(inst);
    }
}

int player_play(const char *url) {
    if (!inst) return -1;
    
    // Stop existing if playing
    if (mp) {
        libvlc_media_player_stop(mp);
        libvlc_media_player_release(mp);
        mp = NULL;
    }

    libvlc_media_t *m = libvlc_media_new_location(inst, url);
    if (!m) {
        fprintf(stderr, "Failed to create media for URL: %s\n", url);
        return -1;
    }

    mp = libvlc_media_player_new_from_media(m);
    libvlc_media_release(m); // Player holds reference now

    if (!mp) {
         fprintf(stderr, "Failed to create media player\n");
         return -1;
    }

    if (libvlc_media_player_play(mp) == -1) {
        fprintf(stderr, "Failed to play media\n");
        return -1;
    }

    return 0;
}

void player_stop() {
    if (mp) {
        libvlc_media_player_stop(mp);
    }
}

char* player_get_metadata() {
    if (!mp) return NULL;
    
    libvlc_media_t *m = libvlc_media_player_get_media(mp);
    if (!m) return NULL;

    // libvlc_media_player_get_media increments refcount? No, usually it doesn't.
    // Wait, documentation says: "The media player holds a reference to the media instance..."
    // But `libvlc_media_player_get_media` returns the media associated with the player.
    // We should parse it just in case metadata isn't parsed yet, but for a stream it might be async.
    // libvlc_media_parse_with_options(m, libvlc_media_parse_local, 0); // Blocking parse might be too slow.
    
    // Try to get NowPlaying
    char *meta = libvlc_media_get_meta(m, libvlc_meta_NowPlaying);
    if (!meta) {
        // Fallback to Title
        meta = libvlc_media_get_meta(m, libvlc_meta_Title);
    }
    
    // We don't release 'm' because we didn't create a new reference, we just got a pointer from mp.
    // ACTUALLY, checking docs: libvlc_media_player_get_media DOES return a new reference.
    // So we must release it.
    libvlc_media_release(m);

    return meta; // Caller frees this
}
