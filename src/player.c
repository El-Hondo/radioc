#include <stdio.h>
#include <stdlib.h>
#include <vlc/vlc.h>
#include "player.h"

static libvlc_instance_t * inst;
static libvlc_media_player_t *mp;

int player_init() {
    inst = libvlc_new(0, NULL);
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
