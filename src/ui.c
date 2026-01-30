#define _XOPEN_SOURCE 600
#include <stdio.h>
#include <string.h>
#include <ctype.h>
#include <wchar.h>
#include <stdlib.h>
#include <locale.h>
#include "ui.h"

// Helper to remove control chars (newlines, tabs) and trim extra whitespace if needed
static void sanitize_str(char *str) {
    char *src = str, *dst = str;
    // Skip leading whitespace
    while (isspace((unsigned char)*src)) src++;
    
    // Copy remainder, replacing control chars with spaces
    for (; *src; src++) {
        if (*src == '\n' || *src == '\r' || *src == '\t') {
            *dst++ = ' ';
        } else {
            *dst++ = *src;
        }
    }
    *dst = '\0';
}

// Helper to count visual width of string (handles UTF-8 and double-width chars)
static int visual_width(const char *s) {
    int w = 0;
    wchar_t wc;
    int len;
    mblen(NULL, 0); // Reset internal state
    
    while (*s) {
        len = mbtowc(&wc, s, 4); // UTF-8 max 4 bytes usually sufficient, or MB_CUR_MAX
        if (len < 0) {
            // Invalid, skip 1 byte
            s++;
            w++; // Assume width 1 for replacement char usually
            mblen(NULL, 0); // Reset
        } else if (len == 0) {
            break; 
        } else {
            int width = wcwidth(wc);
            if (width >= 0) w += width;
            s += len;
        }
    }
    return w;
}

void ui_print_help() {
    printf("\n=== VibeRadio Commands ===\n");
    printf("  find or f <query>   : Find stations by name/tag\n");
    printf("  list or l           : List saved favorites\n");
    printf("  play or p <id>      : Play station from the LAST SHOWN list (search or favs)\n");
    printf("  stop or s           : Stop playback\n");
    printf("  add or a <id>       : Add station from LAST SHOWN list to favorites\n");
    printf("  del or d <id>       : Delete station from FAVORITES list\n");
    printf("  help or ?           : Show this message\n");
    printf("  country or c <id>   : Search for stations by country code (e.g. US, DE)\n");
    printf("  tag or t <tag>      : Search for stations by tag (e.g. jazz, pop)\n");
    printf("  quit or q           : Exit\n");
    printf("==========================\n");
}

void ui_print_credits() {
    printf("\n=== VibeRadio Credits ===\n");
    printf("  Author: El Gringo\n");
    printf("  Version: 0.1\n");
    printf("==========================\n");
}

void ui_print_stations(const Station *stations, int count, const char *title) {

    if (count == 0) {
        printf("\n--- %s ---\n", title);
        printf("No accessible stations found.\n");
        return;
    }

    // Header - votes column widened to 9 characters
    printf("\n┏━━━━━┳━━━━━━━━━━━━━━━━━━━━━━━━━━━━━━━━━━━┳━━━━━━━━━━━━━━━━━━━━━━━━━┳━━━━━━━━━━━━━━━━━━━━━━━━━┳━━━━━━━━━━━┓\n");
    printf("┃ ID  ┃ Station                           ┃ Country                 ┃ Tags                    ┃ votes     ┃\n");
    printf("┡━━━━━╇━━━━━━━━━━━━━━━━━━━━━━━━━━━━━━━━━━━╇━━━━━━━━━━━━━━━━━━━━━━━━━╇━━━━━━━━━━━━━━━━━━━━━━━━━╇━━━━━━━━━━━┩\n");

    for (int i = 0; i < count; i++) {
        char name[256];
        char country[256];
        char tags[256];
        char votes[16];

        // Format and truncate strings
        // We use larger buffers but will check visual width
        
        // Station Name: Max visual width 33
        snprintf(name, sizeof(name), "%s", stations[i].name);
        sanitize_str(name);
        // Truncate if visual width > 33
        while (visual_width(name) > 33) {
             // Simple truncation: remove last bytes until fits. 
             // Ideally we shouldn't cut mid-multibyte.
             // Hack: remove last char
             name[strlen(name)-1] = 0;
             if (visual_width(name) <= 30) { strcat(name, "..."); break; }
        }

        // Country: Max visual width 23
        snprintf(country, sizeof(country), "%s", stations[i].country);
        sanitize_str(country);
        while (visual_width(country) > 23) {
             country[strlen(country)-1] = 0;
             if (visual_width(country) <= 20) { strcat(country, "..."); break; }
        }

        // Tags: Max visual width 23
        snprintf(tags, sizeof(tags), "%s", stations[i].tags);
        sanitize_str(tags);
        while (visual_width(tags) > 23) {
             tags[strlen(tags)-1] = 0;
             if (visual_width(tags) <= 20) { strcat(tags, "..."); break; }
        }

        snprintf(votes, sizeof(votes), "%d", stations[i].votes);

        // Calculate padding
        int pad_name = 33 - visual_width(name);
        if (pad_name < 0) pad_name = 0;
        
        int pad_country = 23 - visual_width(country);
        if (pad_country < 0) pad_country = 0;

        int pad_tags = 23 - visual_width(tags);
        if (pad_tags < 0) pad_tags = 0;
        
        // Votes width is now 9
        // We want left alignment usually for text, but votes is number. 
        // User example looked like left aligned or maybe just whatever. 
        // Let's stick to left alignment as per previous implementation `%-5s`
        // But the header "votes" uses 5 chars.
        // It's a nice column, let's make it 9 chars wide.
        
        printf("│ %-3d │ %s%*s │ %s%*s │ %s%*s │ %-9s │\n", 
               i + 1, 
               name, pad_name, "",
               country, pad_country, "",
               tags, pad_tags, "",
               votes);
    }
    printf("└─────┴───────────────────────────────────┴─────────────────────────┴─────────────────────────┴───────────┘\n");
}

void ui_clear_screen() {
    // ANSI escape code to clear screen and move cursor to top-left
    printf("\033[2J\033[H");
}

void ui_render_interface(const Station *active_station, 
                         const char *song_title, 
                         const char *status_message, 
                         const Station *list, 
                         int list_count, 
                         const char *list_title) {
    
    ui_clear_screen();

    // 1. Draw List if available
    if (list && list_count > 0) {
        ui_print_stations(list, list_count, list_title ? list_title : "Stations");
    }

    // 2. Draw Status Message (if any)
    if (status_message && *status_message) {
        printf("\n>> %s\n", status_message);
    }

    // 3. Draw Persistent "Now Playing" Banner
    if (active_station) {
        printf("\n=========================================================================================\n");
        printf(" ♫ NOW PLAYING: %s\n", active_station->name);
        if (song_title && *song_title) {
            printf(" ♬ INFO:        %s\n", song_title);
        }
        printf("=========================================================================================\n");
    }
}
