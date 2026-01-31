#define _XOPEN_SOURCE 600
#include <stdio.h>
#include <string.h>
#include <ctype.h>
#include <wchar.h>
#include <stdlib.h>
#include <locale.h>
#include "ui.h"

// Global Theme Instance
UITheme current_theme;

void ui_set_theme(const char *theme_name) {
    if (strcmp(theme_name, "tokyo") == 0) {
        // Tokyo Night: Dark Blues, Purples, Cyan
        strcpy(current_theme.name, "tokyo");
        current_theme.border = "\033[38;5;63m";     // Purplish Blue
        current_theme.header = "\033[1;38;5;111m";  // Sky Blue
        current_theme.id     = "\033[38;5;178m";    // Gold
        current_theme.name_col = "\033[1;38;5;255m"; // White
        current_theme.meta   = "\033[38;5;75m";     // Soft Blue
        current_theme.tags   = "\033[38;5;176m";    // Light Pink/Purple
        current_theme.highlight_bg = "\033[48;5;62m";  // Deep Blue/Purple BG
        current_theme.highlight_fg = "\033[1;37m";     // White FG
        current_theme.background   = "";               // Transparent/Default
        current_theme.reset        = "\033[0m";
    }
    else if (strcmp(theme_name, "euro") == 0) {
        // Euro: Official Colours. reflex Blue (21) and Yellow (226)
        strcpy(current_theme.name, "euro");
        current_theme.border = "\033[38;5;226m";      // Yellow
        current_theme.header = "\033[1;38;5;226m";    // Bold Yellow
        current_theme.id     = "\033[38;5;226m";      // Yellow
        current_theme.name_col = "\033[1;37m";        // White
        current_theme.meta   = "\033[37m";            // Dim White
        current_theme.tags   = "\033[38;5;226m";      // Yellow
        current_theme.votes  = "\033[38;5;226m";      // Yellow
        // Selection: Invert (Yellow BG, Blue FG)
        current_theme.highlight_bg = "\033[48;5;226m"; 
        current_theme.highlight_fg = "\033[38;5;21m";  
        // Background: Reflex Blue
        current_theme.background   = "\033[48;5;21m";
        // Reset: Reset all, then re-apply Blue BG
        current_theme.reset        = "\033[0m\033[48;5;21m"; 
    }
    else if (strcmp(theme_name, "matrix") == 0) {
        // Matrix: All Green
        strcpy(current_theme.name, "matrix");
        current_theme.border = COLOR_GREEN;
        current_theme.header = COLOR_BOLD_GREEN;
        current_theme.id     = COLOR_GREEN;
        current_theme.name_col = COLOR_BOLD_WHITE;
        current_theme.meta   = COLOR_GREEN;
        current_theme.tags   = COLOR_DIM;
        current_theme.votes  = COLOR_BOLD_GREEN;
        current_theme.highlight_bg = "\033[7m";
        current_theme.highlight_fg = "";
        current_theme.background   = "";
        current_theme.reset        = "\033[0m";
    }
    else if (strcmp(theme_name, "cyberpunk") == 0) {
         // Cyberpunk: Neon Pink, Yellow, Cyan
        strcpy(current_theme.name, "cyberpunk");
        current_theme.border = COLOR_BOLD_MAGENTA;
        current_theme.header = COLOR_BOLD_YELLOW;
        current_theme.id     = COLOR_BOLD_CYAN;
        current_theme.name_col = COLOR_BOLD_WHITE;
        current_theme.meta   = COLOR_CYAN;
        current_theme.tags   = COLOR_MAGENTA;
        current_theme.votes  = COLOR_YELLOW;
        current_theme.highlight_bg = "\033[45m"; // Magenta BG
        current_theme.highlight_fg = "\033[1;37m";
        current_theme.background   = "";
        current_theme.reset        = "\033[0m";
    }
    else {
        // Default
        strcpy(current_theme.name, "default");
        current_theme.border = COLOR_BLUE;
        current_theme.header = COLOR_BOLD_CYAN;
        current_theme.id     = COLOR_YELLOW;
        current_theme.name_col = COLOR_BOLD_WHITE;
        current_theme.meta   = COLOR_CYAN;
        current_theme.tags   = COLOR_MAGENTA;
        current_theme.votes  = COLOR_GREEN;
        current_theme.highlight_bg = "\033[7m"; // Reverse
        current_theme.highlight_fg = "";
        current_theme.background   = "";
        current_theme.reset        = "\033[0m";
    }
}

void ui_cycle_theme() {
    if (strcmp(current_theme.name, "default") == 0) ui_set_theme("tokyo");
    else if (strcmp(current_theme.name, "tokyo") == 0) ui_set_theme("euro");
    else if (strcmp(current_theme.name, "euro") == 0) ui_set_theme("cyberpunk");
    else if (strcmp(current_theme.name, "cyberpunk") == 0) ui_set_theme("matrix");
    else ui_set_theme("default");
}

void ui_clear_screen() {
    // ANSI escape code to clear screen, move cursor to top-left, and clear scrollback buffer
    // APPLY BACKGROUND COLOR FIRST if set
    if (current_theme.background && *current_theme.background) {
        printf("%s", current_theme.background);
    } else {
        printf("\033[49m"); // Reset background
    }
    printf("\033[H\033[2J\033[3J");
    fflush(stdout);
}

void ui_init() {
    // Default theme init (will be overridden by config usually)
    ui_set_theme("default");
    
    // Enter alternate screen buffer
    printf("\033[?1049h");
    fflush(stdout);
    
    ui_clear_screen();
}

void ui_cleanup() {
    // Reset colors
    printf("\033[0m"); 
    // Leave alternate screen buffer
    printf("\033[?1049l");
    fflush(stdout);
}

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
    printf("\n%s┏━━━━━━━━━━━━━━━━━━━━━━━━━━━━━━━━━━━━━━━━━━━━━━━━━━━━━━━━━━━━━━━━━━━━━━━━━━━━━━━━━━━━━━━━━━━━━━━━━━━━━━━━━┓%s\n", BORDER_COLOR, COLOR_RESET);
    printf("%s┃" COLOR_BOLD_MAGENTA "                                     📻  VibeRadio Commands 📻                                           %s┃%s\n", BORDER_COLOR, BORDER_COLOR, COLOR_RESET);
    printf("%s┡━━━━━━━━━━━━━━━━━━━━━━━━━━━━━┯━━━━━━━━━━━━━━━━━━━━━━━━━━━━━━━━━━━━━━━━━━━━━━━━━━━━━━━━━━━━━━━━━━━━━━━━━━━┩%s\n", BORDER_COLOR, COLOR_RESET);
    printf("%s┃%s Command                     %s┃%s Description                                                               %s┃%s\n", BORDER_COLOR, HEADER_COLOR, BORDER_COLOR, HEADER_COLOR, BORDER_COLOR, COLOR_RESET);
    printf("%s┡━━━━━━━━━━━━━━━━━━━━━━━━━━━━━╇━━━━━━━━━━━━━━━━━━━━━━━━━━━━━━━━━━━━━━━━━━━━━━━━━━━━━━━━━━━━━━━━━━━━━━━━━━━┩%s\n", BORDER_COLOR, COLOR_RESET);
    printf("%s│" COLOR_BOLD_GREEN " find" " or " COLOR_BOLD_GREEN "f" " " COLOR_CYAN "<query>" "           %s│" " Find stations by name/tag                                                 %s│%s\n", BORDER_COLOR, BORDER_COLOR, BORDER_COLOR, COLOR_RESET);
    printf("%s│" COLOR_BOLD_GREEN " list" " or " COLOR_BOLD_GREEN "l" "                   %s│" " List saved favorites                                                      %s│%s\n", BORDER_COLOR, BORDER_COLOR, BORDER_COLOR, COLOR_RESET);
    printf("%s│" COLOR_BOLD_GREEN " play" " or " COLOR_BOLD_GREEN "p" " " COLOR_CYAN "<id>" "              %s│" " Play station from the LAST SHOWN list (search or favs)                    %s│%s\n", BORDER_COLOR, BORDER_COLOR, BORDER_COLOR, COLOR_RESET);
    printf("%s│" COLOR_BOLD_GREEN " stop" " or " COLOR_BOLD_GREEN "s" "                   %s│" " Stop playback                                                             %s│%s\n", BORDER_COLOR, BORDER_COLOR, BORDER_COLOR, COLOR_RESET);
    printf("%s│" COLOR_BOLD_GREEN " add"  " or " COLOR_BOLD_GREEN "a" " " COLOR_CYAN "<id>" "               %s│" " Add station from LAST SHOWN list to favorites                             %s│%s\n", BORDER_COLOR, BORDER_COLOR, BORDER_COLOR, COLOR_RESET);
    printf("%s│" COLOR_BOLD_GREEN " del"  " or " COLOR_BOLD_GREEN "d" " " COLOR_CYAN "<id>" "               %s│" " Delete station from FAVORITES list                                        %s│%s\n", BORDER_COLOR, BORDER_COLOR, BORDER_COLOR, COLOR_RESET);
    printf("%s│" COLOR_BOLD_GREEN " help" " or " COLOR_BOLD_GREEN "?" "                   %s│" " Show this message                                                         %s│%s\n", BORDER_COLOR, BORDER_COLOR, BORDER_COLOR, COLOR_RESET);
    printf("%s│" COLOR_BOLD_GREEN " country" " or " COLOR_BOLD_GREEN "c" " " COLOR_CYAN "<code>" "         %s│" " Search for stations by country code (e.g. US, DE)                         %s│%s\n", BORDER_COLOR, BORDER_COLOR, BORDER_COLOR, COLOR_RESET);
    printf("%s│" COLOR_BOLD_GREEN " tag" " or " COLOR_BOLD_GREEN "t" " " COLOR_CYAN "<tag>" "              %s│" " Search for stations by tag (e.g. jazz, pop)                               %s│%s\n", BORDER_COLOR, BORDER_COLOR, BORDER_COLOR, COLOR_RESET);
    printf("%s│" COLOR_BOLD_GREEN " quit" " or " COLOR_BOLD_GREEN "q" "                   %s│" " Exit                                                                      %s│%s\n", BORDER_COLOR, BORDER_COLOR, BORDER_COLOR, COLOR_RESET);
    printf("%s└─────────────────────────────┴───────────────────────────────────────────────────────────────────────────┘%s\n", BORDER_COLOR, COLOR_RESET);
}

void ui_print_credits() {
    printf("\n%s┏━━━━━━━━━━━━━━━━━━━━━━━━━━━━━━━━━━━━━━━━━━━━━━━━━━━━━━━━━━━━━━━━━━━━━━━━━━━━━━━━━━━━━━━━━━━━━━━━━━━━━━━━━┓%s\n", BORDER_COLOR, COLOR_RESET);
    printf("%s┃" COLOR_BOLD_MAGENTA "                                         ✨ VibeRadio Credits ✨                                         %s┃%s\n", BORDER_COLOR, BORDER_COLOR, COLOR_RESET);
    printf("%s┡━━━━━━━━━━━━━━━━━━━━━━━━━━━━━━━━━━━━━━━━━━━━━━━━━━━━━━━━━━━━━━━━━━━━━━━━━━━━━━━━━━━━━━━━━━━━━━━━━━━━━━━━━┩%s\n", BORDER_COLOR, COLOR_RESET);
    printf("%s┃%s                                                                                                         %s┃%s\n", BORDER_COLOR, HEADER_COLOR, BORDER_COLOR, COLOR_RESET);
    printf("%s┃%s    👨‍💻  Author: " COLOR_BOLD_WHITE "El Gringo" "                                                                              %s┃%s\n", BORDER_COLOR, HEADER_COLOR, BORDER_COLOR, COLOR_RESET);
    printf("%s┃%s    🏷️  Version: " COLOR_CYAN "0.1"  "                                                                                      %s┃%s\n", BORDER_COLOR, HEADER_COLOR, BORDER_COLOR, COLOR_RESET);
    printf("%s┃%s                                                                                                         %s┃%s\n", BORDER_COLOR, HEADER_COLOR, BORDER_COLOR, COLOR_RESET);
    printf("%s┃%s    📧  Contact: " COLOR_BOLD_CYAN "your.email@example.com" "                                                                  %s┃%s\n", BORDER_COLOR, HEADER_COLOR, BORDER_COLOR, COLOR_RESET);
    printf("%s┃%s    🐙  GitHub:  " COLOR_BOLD_BLUE "github.com/your-username/viberadio" "                                                      %s┃%s\n", BORDER_COLOR, HEADER_COLOR, BORDER_COLOR, COLOR_RESET);
    printf("%s┃%s                                                                                                         %s┃%s\n", BORDER_COLOR, HEADER_COLOR, BORDER_COLOR, COLOR_RESET);
    printf("%s┃%s    Made with ❤️  and " COLOR_BOLD_GREEN "C%s & " COLOR_BOLD_MAGENTA "Antigravity%s                                                                     %s┃%s\n", BORDER_COLOR, HEADER_COLOR, COLOR_RESET, COLOR_RESET, BORDER_COLOR, COLOR_RESET);
    printf("%s┃%s                                                                                                         %s┃%s\n", BORDER_COLOR, HEADER_COLOR, BORDER_COLOR, COLOR_RESET);
    printf("%s┗━━━━━━━━━━━━━━━━━━━━━━━━━━━━━━━━━━━━━━━━━━━━━━━━━━━━━━━━━━━━━━━━━━━━━━━━━━━━━━━━━━━━━━━━━━━━━━━━━━━━━━━━━┛%s\n", BORDER_COLOR, COLOR_RESET);
}

void ui_print_stations(const Station *stations, int count, const char *title, int selected_index, int start_index) {

    if (count == 0) {
        printf("\n" COLOR_BOLD_RED "--- %s ---%s\n", title, COLOR_RESET);
        printf(COLOR_RED "No accessible stations found.%s\n", COLOR_RESET);
        return;
    }

    int show_votes = (strcmp(title, "Favorites") != 0);

    if (show_votes) {
        // --- Search Results Layout (With Votes) ---
        // Widths: ID(5) | Votes(7) | Name(25) | Country(18) | Tags(30) => Total 89
        
        printf("\n%s┏━━━━━┳━━━━━━━━━┳━━━━━━━━━━━━━━━━━━━━━━━━━┳━━━━━━━━━━━━━━━━━━┳━━━━━━━━━━━━━━━━━━━━━━━━━━━━━━┓%s\n", BORDER_COLOR, COLOR_RESET);
        printf("%s┃%s ID  %s┃%s  Votes  %s┃%s Station                 %s┃%s Country          %s┃%s Tags                         %s┃%s\n", 
               BORDER_COLOR, HEADER_COLOR, BORDER_COLOR, HEADER_COLOR, BORDER_COLOR, HEADER_COLOR, BORDER_COLOR, HEADER_COLOR, BORDER_COLOR, HEADER_COLOR, BORDER_COLOR, COLOR_RESET);
        printf("%s┡━━━━━╇━━━━━━━━━╇━━━━━━━━━━━━━━━━━━━━━━━━━╇━━━━━━━━━━━━━━━━━━╇━━━━━━━━━━━━━━━━━━━━━━━━━━━━━━┩%s\n", BORDER_COLOR, COLOR_RESET);

        for (int i = 0; i < count; i++) {
            char name[256], country[256], tags[256];
            
            // Truncate to new widths
            snprintf(name, sizeof(name), "%s", stations[i].name);
            sanitize_str(name);
            while (visual_width(name) > 23) { name[strlen(name)-1] = 0; if (visual_width(name)<=20) { strcat(name, "..."); break; } }
            
            snprintf(country, sizeof(country), "%s", stations[i].country);
            sanitize_str(country);
            while (visual_width(country) > 16) { country[strlen(country)-1] = 0; if (visual_width(country)<=13) { strcat(country, "..."); break; } }

            snprintf(tags, sizeof(tags), "%s", stations[i].tags);
            sanitize_str(tags);
            while (visual_width(tags) > 28) { tags[strlen(tags)-1] = 0; if (visual_width(tags)<=25) { strcat(tags, "..."); break; } }

            int pad_name = 23 - visual_width(name); if(pad_name<0) pad_name=0;
            int pad_country = 16 - visual_width(country); if(pad_country<0) pad_country=0;
            int pad_tags = 28 - visual_width(tags); if(pad_tags<0) pad_tags=0;

            if (i + start_index == selected_index) {
                printf("%s│%s%s%s %-3d │ %-7d │ %s%*s │ %s%*s │ %s%*s %s%s│%s\n",
                       BORDER_COLOR, COLOR_RESET, current_theme.highlight_bg, current_theme.highlight_fg,
                       start_index + i + 1, stations[i].votes,
                       name, pad_name, "", country, pad_country, "", tags, pad_tags, "",
                       COLOR_RESET, BORDER_COLOR, COLOR_RESET);
            } else {
                printf("%s│%s %-3d %s│%s %-7d %s│%s %s%*s %s│%s %s%*s %s│%s %s%*s %s│%s\n",
                       BORDER_COLOR, ID_COLOR, start_index + i + 1,
                       BORDER_COLOR, current_theme.votes, stations[i].votes,
                       BORDER_COLOR, NAME_COLOR, name, pad_name, "",
                       BORDER_COLOR, META_COLOR, country, pad_country, "",
                       BORDER_COLOR, TAGS_COLOR, tags, pad_tags, "",
                       BORDER_COLOR, COLOR_RESET);
            }
        }
        printf("%s└─────┴─────────┴─────────────────────────┴──────────────────┴──────────────────────────────┘%s\n", BORDER_COLOR, COLOR_RESET);

    } else {
        // --- Favorites Layout (No Votes) ---
        // Widths: ID(5) | Name(30) | Country(20) | Tags(30) => Total 88
        
        printf("\n%s┏━━━━━┳━━━━━━━━━━━━━━━━━━━━━━━━━━━━━━┳━━━━━━━━━━━━━━━━━━━━┳━━━━━━━━━━━━━━━━━━━━━━━━━━━━━━┓%s\n", BORDER_COLOR, COLOR_RESET);
        printf("%s┃%s ID  %s┃%s Station                      %s┃%s Country            %s┃%s Tags                         %s┃%s\n", BORDER_COLOR, HEADER_COLOR, BORDER_COLOR, HEADER_COLOR, BORDER_COLOR, HEADER_COLOR, BORDER_COLOR, HEADER_COLOR, BORDER_COLOR, COLOR_RESET);
        printf("%s┡━━━━━╇━━━━━━━━━━━━━━━━━━━━━━━━━━━━━━╇━━━━━━━━━━━━━━━━━━━━╇━━━━━━━━━━━━━━━━━━━━━━━━━━━━━━┩%s\n", BORDER_COLOR, COLOR_RESET);

        for (int i = 0; i < count; i++) {
            char name[256], country[256], tags[256];

            snprintf(name, sizeof(name), "%s", stations[i].name);
            sanitize_str(name);
            while (visual_width(name) > 28) { name[strlen(name)-1] = 0; if (visual_width(name)<=25) { strcat(name, "..."); break; } }

            snprintf(country, sizeof(country), "%s", stations[i].country);
            sanitize_str(country);
            while (visual_width(country) > 18) { country[strlen(country)-1] = 0; if (visual_width(country)<=15) { strcat(country, "..."); break; } }

            snprintf(tags, sizeof(tags), "%s", stations[i].tags);
            sanitize_str(tags);
            while (visual_width(tags) > 28) { tags[strlen(tags)-1] = 0; if (visual_width(tags)<=25) { strcat(tags, "..."); break; } }

            int pad_name = 28 - visual_width(name); if(pad_name<0) pad_name=0;
            int pad_country = 18 - visual_width(country); if(pad_country<0) pad_country=0;
            int pad_tags = 28 - visual_width(tags); if(pad_tags<0) pad_tags=0;

            if (i + start_index == selected_index) {
                printf("%s│%s%s%s %-3d │ %s%*s │ %s%*s │ %s%*s  %s%s│%s\n",
                       BORDER_COLOR, COLOR_RESET, current_theme.highlight_bg, current_theme.highlight_fg,
                       start_index + i + 1, 
                       name, pad_name, "", country, pad_country, "", tags, pad_tags, "",
                       COLOR_RESET, BORDER_COLOR, COLOR_RESET);
            } else {
                printf("%s│%s %-3d %s│%s %s%*s %s│%s %s%*s %s│%s %s%*s %s│%s\n", 
                       BORDER_COLOR, ID_COLOR, start_index + i + 1, 
                       BORDER_COLOR, NAME_COLOR, name, pad_name, "",
                       BORDER_COLOR, META_COLOR, country, pad_country, "",
                       BORDER_COLOR, TAGS_COLOR, tags, pad_tags, "",
                       BORDER_COLOR, COLOR_RESET);
            }
        }
        printf("%s└─────┴──────────────────────────────┴────────────────────┴──────────────────────────────┘%s\n", BORDER_COLOR, COLOR_RESET);
    }
}

void ui_render_interface(const Station *active_station, 
                         const char *song_title, 
                         const char *status_message, 
                         const Station *list, 
                         int list_count, 
                         const char *list_title,
                         int selected_index) {
    
    ui_clear_screen();

    // 1. Draw List if available
    if (list && list_count > 0) {
        int page_size = 25;
        // Determine start_index based on selected_index
        // We want selected_index to be visible.
        // Simple Logic: Page = selected_index / page_size
        int current_page = (selected_index >= 0) ? (selected_index / page_size) : 0;
        int start_index = current_page * page_size;
        
        // Count to show on this page
        int count_on_page = page_size;
        if (start_index + count_on_page > list_count) {
            count_on_page = list_count - start_index;
        }
        
        // Modify title to include page info?
        char paged_title[256];
        int total_pages = (list_count + page_size - 1) / page_size;
        snprintf(paged_title, sizeof(paged_title), "%s (Page %d/%d)", list_title ? list_title : "Stations", current_page + 1, total_pages);

        // Pass pointer to start of page
        ui_print_stations(&list[start_index], count_on_page, paged_title, selected_index, start_index);
    }

    // 2. Draw Status Message (if any)
    if (status_message && *status_message) {
        printf("\n" COLOR_BOLD_YELLOW ">> %s%s\n", status_message, COLOR_RESET);
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

        printf("\n%s╔═════════════════════════════════════════════════════════════════════════════════════════════════════════╗%s\n", BORDER_COLOR, COLOR_RESET);
        printf("%s║%s ♫  " COLOR_BOLD_CYAN "NOW PLAYING:" COLOR_BOLD_WHITE " %s%*s %s║%s\n", BORDER_COLOR, COLOR_RESET, display_name, pad_name, "", BORDER_COLOR, COLOR_RESET);
        
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
            printf("%s║%s ♬  " COLOR_CYAN "INFO:       " COLOR_WHITE " %s%*s %s║%s\n", BORDER_COLOR, COLOR_RESET, display_title, pad_title, "", BORDER_COLOR, COLOR_RESET);
        }
        printf("%s╚═════════════════════════════════════════════════════════════════════════════════════════════════════════╝%s\n", BORDER_COLOR, COLOR_RESET);
    }
}
