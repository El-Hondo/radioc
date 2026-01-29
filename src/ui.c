#include <stdio.h>
#include <string.h>
#include "ui.h"

#include <ctype.h>
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

// Helper to count visual characters (simplified UTF-8: counts non-continuation bytes)
static int utf8_width(const char *s) {
    int w = 0;
    while (*s) {
        if ((*s & 0xC0) != 0x80) w++; // Not a continuation byte
        s++;
    }
    return w;
}

void ui_print_help() {
    printf("\n=== VibeRadio Commands ===\n");
    printf("  search or a <query> : Search for stations by name/tag\n");
    printf("  list or l           : List saved favorites\n");
    printf("  play or p <id>      : Play station from the LAST SHOWN list (search or favs)\n");
    printf("  stop or st          : Stop playback\n");
    printf("  add or ad <id>      : Add station from LAST SHOWN list to favorites\n");
    printf("  del or dl <id>      : Delete station from FAVORITES list\n");
    printf("  help or ?           : Show this message\n");
    printf("  country or co <id>  : Search for stations by country code (e.g. US, DE)\n");
    printf("  quit or q           : Exit\n");
    printf("==========================\n");
}

void ui_print_stations(const Station *stations, int count, const char *title) {
    if (count == 0) {
        printf("\n--- %s ---\n", title);
        printf("No accessible stations found.\n");
        return;
    }

    // Header
    printf("\n┏━━━━━┳━━━━━━━━━━━━━━━━━━━━━━━━━━━━━━━━━━━┳━━━━━━━━━━━━━━━━━━━━━━━━━┳━━━━━━━━━━━━━━━━━━━━━━━━━┳━━━━━━━┓\n");
    printf("┃ ID  ┃ Station                           ┃ Country                 ┃ Tags                    ┃ votes ┃\n");
    printf("┡━━━━━╇━━━━━━━━━━━━━━━━━━━━━━━━━━━━━━━━━━━╇━━━━━━━━━━━━━━━━━━━━━━━━━╇━━━━━━━━━━━━━━━━━━━━━━━━━╇━━━━━━━┩\n");

    for (int i = 0; i < count; i++) {
        char name[34];
        char country[24];
        char tags[24];
        char votes[8];

        // Format and truncate strings
        snprintf(name, sizeof(name), "%s", stations[i].name);
        sanitize_str(name);
        if (strlen(name) > 33) strcpy(name + 30, "...");
        
        snprintf(country, sizeof(country), "%s", stations[i].country);
        sanitize_str(country);
        if (strlen(country) > 23) strcpy(country + 20, "...");

        snprintf(tags, sizeof(tags), "%s", stations[i].tags);
        sanitize_str(tags);
        if (strlen(tags) > 23) strcpy(tags + 20, "...");

        snprintf(votes, sizeof(votes), "%d", stations[i].votes);

        // Calculate padding
        int pad_name = 33 - utf8_width(name);
        if (pad_name < 0) pad_name = 0;
        
        int pad_country = 23 - utf8_width(country);
        if (pad_country < 0) pad_country = 0;

        int pad_tags = 23 - utf8_width(tags);
        if (pad_tags < 0) pad_tags = 0;

        printf("│ %-3d │ %s%*s │ %s%*s │ %s%*s │ %-5s │\n", 
               i + 1, 
               name, pad_name, "",
               country, pad_country, "",
               tags, pad_tags, "",
               votes);
    }
    printf("└─────┴───────────────────────────────────┴─────────────────────────┴─────────────────────────┴───────┘\n");
}
