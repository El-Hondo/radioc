#define _XOPEN_SOURCE 600
#include <stdio.h>
#include <string.h>
#include <ctype.h>
#include <wchar.h>
#include <stdlib.h>
#include <locale.h>
#include "ui.h"

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

// 105 chars of internal width: 29 for Command, 75 for Description, 1 for separator. Total 105.
void ui_print_help() {
    printf("\n" BORDER_COLOR "┏━━━━━━━━━━━━━━━━━━━━━━━━━━━━━━━━━━━━━━━━━━━━━━━━━━━━━━━━━━━━━━━━━━━━━━━━━━━━━━━━━━━━━━━━━━━━━━━━━━━━━━━━━┓" COLOR_RESET "\n");
    printf(BORDER_COLOR "┃" COLOR_BOLD_MAGENTA "                                        VibeRadio Commands                                               " BORDER_COLOR "┃" COLOR_RESET "\n");
    printf(BORDER_COLOR "┡━━━━━━━━━━━━━━━━━━━━━━━━━━━━━┯━━━━━━━━━━━━━━━━━━━━━━━━━━━━━━━━━━━━━━━━━━━━━━━━━━━━━━━━━━━━━━━━━━━━━━━━━━━┩" COLOR_RESET "\n");
    printf(BORDER_COLOR "┃" HEADER_COLOR " Command                     " BORDER_COLOR "┃" HEADER_COLOR " Description                                                               " BORDER_COLOR "┃" COLOR_RESET "\n");
    printf(BORDER_COLOR "┡━━━━━━━━━━━━━━━━━━━━━━━━━━━━━╇━━━━━━━━━━━━━━━━━━━━━━━━━━━━━━━━━━━━━━━━━━━━━━━━━━━━━━━━━━━━━━━━━━━━━━━━━━━┩" COLOR_RESET "\n");
    printf(BORDER_COLOR "│" COLOR_BOLD_GREEN " find" COLOR_RESET " or " COLOR_BOLD_GREEN "f" COLOR_RESET " " COLOR_CYAN "<query>" COLOR_RESET "           " BORDER_COLOR "│" COLOR_RESET " Find stations by name/tag                                                 " BORDER_COLOR "│" COLOR_RESET "\n");
    printf(BORDER_COLOR "│" COLOR_BOLD_GREEN " list" COLOR_RESET " or " COLOR_BOLD_GREEN "l" COLOR_RESET "                   " BORDER_COLOR "│" COLOR_RESET " List saved favorites                                                      " BORDER_COLOR "│" COLOR_RESET "\n");
    printf(BORDER_COLOR "│" COLOR_BOLD_GREEN " play" COLOR_RESET " or " COLOR_BOLD_GREEN "p" COLOR_RESET " " COLOR_CYAN "<id>" COLOR_RESET "              " BORDER_COLOR "│" COLOR_RESET " Play station from the LAST SHOWN list (search or favs)                    " BORDER_COLOR "│" COLOR_RESET "\n");
    printf(BORDER_COLOR "│" COLOR_BOLD_GREEN " stop" COLOR_RESET " or " COLOR_BOLD_GREEN "s" COLOR_RESET "                   " BORDER_COLOR "│" COLOR_RESET " Stop playback                                                             " BORDER_COLOR "│" COLOR_RESET "\n");
    printf(BORDER_COLOR "│" COLOR_BOLD_GREEN " add" COLOR_RESET " or " COLOR_BOLD_GREEN "a" COLOR_RESET " " COLOR_CYAN "<id>" COLOR_RESET "               " BORDER_COLOR "│" COLOR_RESET " Add station from LAST SHOWN list to favorites                             " BORDER_COLOR "│" COLOR_RESET "\n");
    printf(BORDER_COLOR "│" COLOR_BOLD_GREEN " del" COLOR_RESET " or " COLOR_BOLD_GREEN "d" COLOR_RESET " " COLOR_CYAN "<id>" COLOR_RESET "               " BORDER_COLOR "│" COLOR_RESET " Delete station from FAVORITES list                                        " BORDER_COLOR "│" COLOR_RESET "\n");
    printf(BORDER_COLOR "│" COLOR_BOLD_GREEN " help" COLOR_RESET " or " COLOR_BOLD_GREEN "?" COLOR_RESET "                   " BORDER_COLOR "│" COLOR_RESET " Show this message                                                         " BORDER_COLOR "│" COLOR_RESET "\n");
    printf(BORDER_COLOR "│" COLOR_BOLD_GREEN " country" COLOR_RESET " or " COLOR_BOLD_GREEN "c" COLOR_RESET " " COLOR_CYAN "<code>" COLOR_RESET "         " BORDER_COLOR "│" COLOR_RESET " Search for stations by country code (e.g. US, DE)                         " BORDER_COLOR "│" COLOR_RESET "\n");
    printf(BORDER_COLOR "│" COLOR_BOLD_GREEN " tag" COLOR_RESET " or " COLOR_BOLD_GREEN "t" COLOR_RESET " " COLOR_CYAN "<tag>" COLOR_RESET "              " BORDER_COLOR "│" COLOR_RESET " Search for stations by tag (e.g. jazz, pop)                               " BORDER_COLOR "│" COLOR_RESET "\n");
    printf(BORDER_COLOR "│" COLOR_BOLD_GREEN " quit" COLOR_RESET " or " COLOR_BOLD_GREEN "q" COLOR_RESET "                   " BORDER_COLOR "│" COLOR_RESET " Exit                                                                      " BORDER_COLOR "│" COLOR_RESET "\n");
    printf(BORDER_COLOR "└─────────────────────────────┴───────────────────────────────────────────────────────────────────────────┘" COLOR_RESET "\n");
}

void ui_print_credits() {
    printf("\n" COLOR_BOLD_MAGENTA "=== VibeRadio Credits ===" COLOR_RESET "\n");
    printf("  Author: " COLOR_BOLD_WHITE "El Gringo" COLOR_RESET "\n");
    printf("  Version: " COLOR_CYAN "0.1" COLOR_RESET "\n");
    printf(COLOR_BOLD_MAGENTA "==========================" COLOR_RESET "\n");

    printf("\n" BORDER_COLOR                  "┏━━━━━━━━━━━━━━━━━━━━━━━━━━━━━━━━━━━━━━━━━━━━━━━━━━━━━━━━━━━━━━━━━━━━━━━━━━━━━━━━━━━━━━━━━━━━━━━━━━━━━━━━━┓" COLOR_RESET "\n");
    printf(BORDER_COLOR "┃" COLOR_BOLD_MAGENTA "                                           VibeRadio Credits                                             " BORDER_COLOR "┃" COLOR_RESET "\n");
    printf(BORDER_COLOR "┃" HEADER_COLOR       "         Author: " COLOR_BOLD_WHITE "El Gringo" BORDER_COLOR "                                                                               ┃" COLOR_RESET "\n");
    printf(BORDER_COLOR "┃" HEADER_COLOR       "         Version: " COLOR_CYAN "0.1"  BORDER_COLOR "                                                                                    ┃" COLOR_RESET "\n");
    printf(BORDER_COLOR "┃" HEADER_COLOR       "                                                                                                         ┃" COLOR_RESET "\n");
    printf(BORDER_COLOR "┡━━━━━━━━━━━━━━━━━━━━━━━━━━━━━━━━━━━━━━━━━━━━━━━━━━━━━━━━━━━━━━━━━━━━━━━━━━━━━━━━━━━━━━━━━━━━━━━━━━━━━━━━━┩" COLOR_RESET "\n");
}

void ui_print_stations(const Station *stations, int count, const char *title) {

    if (count == 0) {
        printf("\n" COLOR_BOLD_RED "--- %s ---" COLOR_RESET "\n", title);
        printf(COLOR_RED "No accessible stations found." COLOR_RESET "\n");
        return;
    }

    // Header - votes column widened to 9 characters
    // Header - tags column widened to 37 characters (merged with votes)
    printf("\n" BORDER_COLOR "┏━━━━━┳━━━━━━━━━━━━━━━━━━━━━━━━━━━━━━━━━━━┳━━━━━━━━━━━━━━━━━━━━━━━━━┳━━━━━━━━━━━━━━━━━━━━━━━━━━━━━━━━━━━━━┓" COLOR_RESET "\n");
    printf(BORDER_COLOR "┃" HEADER_COLOR " ID  " BORDER_COLOR "┃" HEADER_COLOR " Station                           " BORDER_COLOR "┃" HEADER_COLOR " Country                 " BORDER_COLOR "┃" HEADER_COLOR " Tags                                " BORDER_COLOR "┃" COLOR_RESET "\n");
    printf(BORDER_COLOR "┡━━━━━╇━━━━━━━━━━━━━━━━━━━━━━━━━━━━━━━━━━━╇━━━━━━━━━━━━━━━━━━━━━━━━━╇━━━━━━━━━━━━━━━━━━━━━━━━━━━━━━━━━━━━━┩" COLOR_RESET "\n");

    for (int i = 0; i < count; i++) {
        char name[256];
        char country[256];
        char tags[256];

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

        // Tags: Max visual width 35 (expanded)
        snprintf(tags, sizeof(tags), "%s", stations[i].tags);
        sanitize_str(tags);
        while (visual_width(tags) > 35) {
             tags[strlen(tags)-1] = 0;
             if (visual_width(tags) <= 32) { strcat(tags, "..."); break; }
        }

        // Calculate padding
        int pad_name = 33 - visual_width(name);
        if (pad_name < 0) pad_name = 0;
        
        int pad_country = 23 - visual_width(country);
        if (pad_country < 0) pad_country = 0;

        int pad_tags = 35 - visual_width(tags);
        if (pad_tags < 0) pad_tags = 0;
        
        printf(BORDER_COLOR "│" ID_COLOR " %-3d " BORDER_COLOR "│" NAME_COLOR " %s%*s " BORDER_COLOR "│" META_COLOR " %s%*s " BORDER_COLOR "│" TAGS_COLOR " %s%*s " BORDER_COLOR "│" COLOR_RESET "\n", 
               i + 1, 
               name, pad_name, "",
               country, pad_country, "",
               tags, pad_tags, "");
    }
    printf(BORDER_COLOR "└─────┴───────────────────────────────────┴─────────────────────────┴─────────────────────────────────────┘" COLOR_RESET "\n");
}

void ui_init() {
    // Enter alternate screen buffer
    printf("\033[?1049h");
    fflush(stdout);
}

void ui_cleanup() {
    // Leave alternate screen buffer
    printf("\033[?1049l");
    fflush(stdout);
}

void ui_clear_screen() {
    // ANSI escape code to clear screen, move cursor to top-left, and clear scrollback buffer
    printf("\033[H\033[2J\033[3J");
    fflush(stdout);
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
        printf("\n" COLOR_BOLD_YELLOW ">> %s" COLOR_RESET "\n", status_message);
    }

    // 3. Draw Persistent "Now Playing" Banner
    if (active_station) {
        // Calculate target width inside the box
        // Total box width = 107. Borders = 2. Content = 105.
        // Label " ♫  NOW PLAYING: " is 17 chars (1+1+2+12+1)
        // Trailing space = 1.
        // Max name width = 105 - 17 - 1 = 87.
        
        char display_name[256];
        snprintf(display_name, sizeof(display_name), "%s", active_station->name);
        sanitize_str(display_name);
        while (visual_width(display_name) > 87) {
             display_name[strlen(display_name)-1] = 0;
             if (visual_width(display_name) <= 84) { strcat(display_name, "..."); break; }
        }
        int pad_name = 87 - visual_width(display_name);
        if (pad_name < 0) pad_name = 0;

        printf("\n" COLOR_BOLD_MAGENTA "╔═════════════════════════════════════════════════════════════════════════════════════════════════════════╗" COLOR_RESET "\n");
        printf(COLOR_BOLD_MAGENTA "║" COLOR_RESET " ♫  " COLOR_BOLD_CYAN "NOW PLAYING:" COLOR_BOLD_WHITE " %s%*s " COLOR_BOLD_MAGENTA "║" COLOR_RESET "\n", display_name, pad_name, "");
        
        if (song_title && *song_title) {
            char display_title[256];
            snprintf(display_title, sizeof(display_title), "%s", song_title);
            sanitize_str(display_title);
            while (visual_width(display_title) > 87) {
                 display_title[strlen(display_title)-1] = 0;
                 if (visual_width(display_title) <= 84) { strcat(display_title, "..."); break; }
            }
            int pad_title = 87 - visual_width(display_title);
            if (pad_title < 0) pad_title = 0;
            
            // Align "INFO:" to match "NOW PLAYING:" length (12 chars) -> "INFO:       "
            printf(COLOR_BOLD_MAGENTA "║" COLOR_RESET " ♬  " COLOR_CYAN "INFO:       " COLOR_WHITE " %s%*s " COLOR_BOLD_MAGENTA "║" COLOR_RESET "\n", display_title, pad_title, "");
        }
        printf(COLOR_BOLD_MAGENTA "╚═════════════════════════════════════════════════════════════════════════════════════════════════════════╝" COLOR_RESET "\n");
    }
}
