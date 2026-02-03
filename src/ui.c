/**
 * @file ui.c
 * @brief TUI rendering and layout logic.
 */
#define _XOPEN_SOURCE 600
#include <stdio.h>
#include <string.h>
#include <ctype.h>
#include <wchar.h>
#include <stdlib.h>
#include <locale.h>
#include <locale.h>
#include <sys/ioctl.h>
#include <unistd.h>
#include "ui.h"


// Global Theme Instance
UITheme current_theme;

void ui_get_term_size(int *rows, int *cols) {
    struct winsize ws;
    if (ioctl(STDOUT_FILENO, TIOCGWINSZ, &ws) == -1 || ws.ws_col == 0) {
        // Fallback
        *cols = 80;
        *rows = 24;
    } else {
        *cols = ws.ws_col;
        *rows = ws.ws_row;
    }
}


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
    // Move to home, ensure background
    printf("\033[H");
    if (current_theme.background && *current_theme.background) {
         printf("%s", current_theme.background);
    } else {
         printf("\033[49m");
    }

    printf("%s┏━━━━━━━━━━━━━━━━━━━━━━━━━━━━━━━━━━━━━━━━━━━━━━━━━━━━━━━━━━━━━━━━━━━━━━━━━━━━━━━━━━━━━━━━━━━━━━━━━━━━━━━━━┓%s\n", BORDER_COLOR, COLOR_RESET);
    printf("%s┃" COLOR_BOLD_MAGENTA "                                     📻  radio.c Commands 📻                                             %s┃%s\n", BORDER_COLOR, BORDER_COLOR, COLOR_RESET);
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
    
    // Clear rest of screen
    printf("\033[J");
}

void ui_print_credits() {
    // Move to home, ensure background
    printf("\033[H");
    if (current_theme.background && *current_theme.background) {
         printf("%s", current_theme.background);
    } else {
         printf("\033[49m");
    }

    printf("%s┏━━━━━━━━━━━━━━━━━━━━━━━━━━━━━━━━━━━━━━━━━━━━━━━━━━━━━━━━━━━━━━━━━━━━━━━━━━━━━━━━━━━━━━━━━━━━━━━━━━━━━━━━━┓%s\n", BORDER_COLOR, COLOR_RESET);
    printf("%s┃" COLOR_BOLD_MAGENTA "                                         ✨ radio.c Credits ✨                                           %s┃%s\n", BORDER_COLOR, BORDER_COLOR, COLOR_RESET);
    printf("%s┡━━━━━━━━━━━━━━━━━━━━━━━━━━━━━━━━━━━━━━━━━━━━━━━━━━━━━━━━━━━━━━━━━━━━━━━━━━━━━━━━━━━━━━━━━━━━━━━━━━━━━━━━━┩%s\n", BORDER_COLOR, COLOR_RESET);
    printf("%s┃%s                                                                                                         %s┃%s\n", BORDER_COLOR, HEADER_COLOR, BORDER_COLOR, COLOR_RESET);
    printf("%s┃%s    👨‍💻  Author: " COLOR_BOLD_WHITE "El Hondo" "                                                                              %s┃%s\n", BORDER_COLOR, HEADER_COLOR, BORDER_COLOR, COLOR_RESET);
    printf("%s┃%s    🏷️  Version: " COLOR_CYAN "0.1"  "                                                                                      %s┃%s\n", BORDER_COLOR, HEADER_COLOR, BORDER_COLOR, COLOR_RESET);
    printf("%s┃%s                                                                                                         %s┃%s\n", BORDER_COLOR, HEADER_COLOR, BORDER_COLOR, COLOR_RESET);
    printf("%s┃%s    📧  Contact: " COLOR_BOLD_CYAN "your.email@example.com" "                                                                  %s┃%s\n", BORDER_COLOR, HEADER_COLOR, BORDER_COLOR, COLOR_RESET);
    printf("%s┃%s    🐙  GitHub:  " COLOR_BOLD_BLUE "github.com/your-username/radioc" "                                                      %s┃%s\n", BORDER_COLOR, HEADER_COLOR, BORDER_COLOR, COLOR_RESET);
    printf("%s┃%s                                                                                                         %s┃%s\n", BORDER_COLOR, HEADER_COLOR, BORDER_COLOR, COLOR_RESET);
    printf("%s┃%s    Made with ❤️  and " COLOR_BOLD_GREEN "C%s & " COLOR_BOLD_MAGENTA "Antigravity%s                                                                     %s┃%s\n", BORDER_COLOR, HEADER_COLOR, COLOR_RESET, COLOR_RESET, BORDER_COLOR, COLOR_RESET);
    printf("%s┃%s                                                                                                         %s┃%s\n", BORDER_COLOR, HEADER_COLOR, BORDER_COLOR, COLOR_RESET);
    printf("%s┗━━━━━━━━━━━━━━━━━━━━━━━━━━━━━━━━━━━━━━━━━━━━━━━━━━━━━━━━━━━━━━━━━━━━━━━━━━━━━━━━━━━━━━━━━━━━━━━━━━━━━━━━━┛%s\n", BORDER_COLOR, COLOR_RESET);
    
    // Clear rest of screen
    printf("\033[J");
}

void ui_print_stations(const Station *stations, int count, const char *title, int selected_index, int start_index) {
    if (count == 0) {
        printf("\n" COLOR_BOLD_RED "--- %s ---%s\n", title, COLOR_RESET);
        printf(COLOR_RED "No accessible stations found.%s\n", COLOR_RESET);
        return;
    }

    int rows, cols;
    ui_get_term_size(&rows, &cols);
    
    // Safety margin
    int total_width = cols - 1; // User requested "one more" (from -2) -> -1
    if (total_width < 40) total_width = 40; // Hard min

    // --- Responsive logic ---
    // Fixed columns: ID (5), Votes (7) [optional]
    // Borders: Left(1) + ID(5) + Sep(1) + [Votes(7)+Sep(1)] + Name(X) + Sep(1) + [Country(Y)+Sep(1) + Tags(Z)+Sep(1)] 
    // Simplified: 
    // Base overhead (min view): Left(1) + ID(5) + Sep(1) + Sep(1) + Right(1) = 9 chars overhead for ID box and ends.
    // Wait, let's look at structure:
    // ┃ ID  ┃ Votes ┃ Station ... ┃ 
    
    int show_votes = (strcmp(title, "Favorites") != 0);
    int show_country = 1;
    int show_tags = 1;

    // Thresholds
    if (total_width < 90) show_tags = 0;
    if (total_width < 60) show_country = 0;

    // Calculate Dynamic Widths
    // ID: 5 fixed
    // Votes: 7 fixed (if shown)
    // Country: 20 target (if shown)
    // Tags: 30 target (if shown)
    // Name: Remainder

    int w_id = 5;
    // w_votes removed (unused), calculated directly below
    // Border overhead logic:
    // Border overhead logic:
    // Left(1) + ID(5) + Sep(1) + [Votes(7) + Sep(1)] + Name(W1) + Sep(1) + [Country(W2) + Sep(1)] + [Tags(W3) + Sep(1)] + Right(0, newline)
    
    int used = 1 + w_id + 1; // Left border + ID + separator
    if (show_votes) used += 7 + 1; // Votes + separator
    if (show_country) used += 1; // Separator
    if (show_tags) used += 1; // Separator
    used += 1; // Right border (effectively)
    
    // Account for padding in variable columns (+2 chars each for ' ' left/right)
    used += 2; // For Name
    if (show_country) used += 2;
    if (show_tags) used += 2;
    
    int available = total_width - used;
    
    // Distribute available
    int w_country = 0;
    int w_tags = 0;
    int w_name = 0;

    if (show_tags && show_country) {
        w_country = 18;
        w_tags = 28;
    } else if (show_country) {
        w_country = 22; // More space for country if tags invalid
    }
    
    // Name gets the rest
    w_name = available - w_country - w_tags;
    
    // Sanity / Minimums checks
    if (w_name < 15) {
        // Force name min, eat country
        if (show_country) {
            w_country = 0;
            show_country = 0;
            used -= 1; // Remove separator
            w_name = available + 18; // Reclaim country space
        }
    }
    
    // Header Generation
    printf("\n%s┏━━━━━", BORDER_COLOR);
    if(show_votes) printf("┳━━━━━━━");
    
    // Name Header
    printf("┳");
    for(int k=0; k<w_name+2;k++) printf("━"); // +2 for padding spaces?
    
    if(show_country) {
        printf("┳");
        for(int k=0; k<w_country+2;k++) printf("━");
    }
    if(show_tags) {
        printf("┳");
        for(int k=0; k<w_tags+2;k++) printf("━");
    }
    printf("┓%s\n", COLOR_RESET);

    // Header Text
    printf("%s┃%s ID  %s", BORDER_COLOR, HEADER_COLOR, BORDER_COLOR);
    if(show_votes) printf("┃%s Votes %s", HEADER_COLOR, BORDER_COLOR);
    
    printf("┃%s %-*s %s", HEADER_COLOR, w_name, "Station", BORDER_COLOR);
    if(show_country) printf("┃%s %-*s %s", HEADER_COLOR, w_country, "Country", BORDER_COLOR);
    if(show_tags) printf("┃%s %-*s %s", HEADER_COLOR, w_tags, "Tags", BORDER_COLOR);
    printf("┃%s\n", COLOR_RESET);

    // Header Separator
    printf("%s┡━━━━━", BORDER_COLOR);
    if(show_votes) printf("╇━━━━━━━");
    printf("╇");
    for(int k=0; k<w_name+2;k++) printf("━");
    if(show_country) { printf("╇"); for(int k=0; k<w_country+2;k++) printf("━"); }
    if(show_tags) { printf("╇"); for(int k=0; k<w_tags+2;k++) printf("━"); }
    printf("┩%s\n", COLOR_RESET);

    // Rows
    for (int i = 0; i < count; i++) {
        char name[256], country[256], tags[256];
        
        // Truncate
        snprintf(name, sizeof(name), "%s", stations[i].name); sanitize_str(name);
        while (visual_width(name) > w_name) { name[strlen(name)-1] = 0; if(visual_width(name) <= w_name-3) { strcat(name, "..."); break; } }
        int pad_name = w_name - visual_width(name); if(pad_name<0) pad_name=0;

        if (show_country) {
             snprintf(country, sizeof(country), "%s", stations[i].country); sanitize_str(country);
             while (visual_width(country) > w_country) { country[strlen(country)-1] = 0; if(visual_width(country) <= w_country-3) { strcat(country, "..."); break; } }
        }
        int pad_country = show_country ? (w_country - visual_width(country)) : 0; if(pad_country<0) pad_country=0;

        if (show_tags) {
             snprintf(tags, sizeof(tags), "%s", stations[i].tags); sanitize_str(tags);
             while (visual_width(tags) > w_tags) { tags[strlen(tags)-1] = 0; if(visual_width(tags) <= w_tags-3) { strcat(tags, "..."); break; } }
        }
        int pad_tags = show_tags ? (w_tags - visual_width(tags)) : 0; if(pad_tags<0) pad_tags=0;


        if (i + start_index == selected_index) {
             // Highlighted Row
             printf("%s│%s%s%s %-3d ", BORDER_COLOR, COLOR_RESET, current_theme.highlight_bg, current_theme.highlight_fg, start_index+i+1);
             if(show_votes) printf("│ %-5d ", stations[i].votes);
             printf("│ %s%*s ", name, pad_name, "");
             if(show_country) printf("│ %s%*s ", country, pad_country, "");
             if(show_tags) printf("│ %s%*s ", tags, pad_tags, "");
             printf("%s%s│%s\n", COLOR_RESET, BORDER_COLOR, COLOR_RESET);
        } else {
             // Normal Row
             printf("%s│%s %-3d %s", BORDER_COLOR, ID_COLOR, start_index+i+1, BORDER_COLOR);
             if(show_votes) printf("│%s %-5d %s", current_theme.votes, stations[i].votes, BORDER_COLOR);
             printf("│%s %s%*s %s", NAME_COLOR, name, pad_name, "", BORDER_COLOR);
             if(show_country) printf("│%s %s%*s %s", META_COLOR, country, pad_country, "", BORDER_COLOR);
             if(show_tags) printf("│%s %s%*s %s", TAGS_COLOR, tags, pad_tags, "", BORDER_COLOR);
             printf("│%s\n", COLOR_RESET);
        }
    }
    
    // Bottom
    printf("%s└─────", BORDER_COLOR);
    if(show_votes) printf("┴───────");
    printf("┴");
    for(int k=0; k<w_name+2;k++) printf("─");
    if(show_country) { printf("┴"); for(int k=0; k<w_country+2;k++) printf("─"); }
    if(show_tags) { printf("┴"); for(int k=0; k<w_tags+2;k++) printf("─"); }
    printf("┘%s\n", COLOR_RESET);
}

// Stub specifically for ui_render_interface to force-match the signature
void ui_render_interface_legacy_stub_placeholder(void) {
    // This function content is replaced by the actual ui_render_interface below
}

void ui_render_interface(const Station *active_station, 
                          const char *song_title, 
                          const char *status_message, 
                          const Station *list, 
                          int list_count, 
                          const char *list_title,
                          int selected_index) {
    
    ui_clear_screen(); // Re-enabled to fix artifacts
    // Move cursor to top-left and ensure background is set
    printf("\033[H");
    if (current_theme.background && *current_theme.background) {
        printf("%s", current_theme.background);
    } else {
        printf("\033[49m");
    }

    // 1. Draw List if available
    if (list && list_count > 0) {
        int rows, cols;
        ui_get_term_size(&rows, &cols);

        // Calculate available vertical space
        // Overhead Estimate:
        // - 1 (Title/Sep)
        // - 3 (Header with borders)
        // - 1 (Bottom border)
        // - 2 (Status area)
        // - 6 (Banner area if active, approx)
        // Total ~13-15 lines. Using 15 to be safe.
        int overhead = 15;
        int page_size = rows - overhead;
        if (page_size < 5) page_size = 5; // Minimum 5 items to keep UI sane
        
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
        printf("\n\033[K" COLOR_BOLD_YELLOW ">> %s%s\n", status_message, COLOR_RESET);
    }

    // 3. Draw Persistent "Now Playing" Banner
    if (active_station) {
        // Variable Width Configuration
        int rows, cols;
        ui_get_term_size(&rows, &cols);
        int banner_width = cols - 1; // Margin matched to table (was -4)
        if (banner_width < 40) banner_width = 40; // Soft floor to prevent absolute implosion, but small enough to fit most "phone" term sizes
        // Removed max width limit to allow full expansion

        int content_width = banner_width - 2; // Width inside the borders
        
        // Label " ♫  NOW PLAYING: " is 17 chars (1+1+2+12+1)
        // Trailing space = 1.
        int label_len = 17;
        int max_text_width = content_width - label_len - 1;
        
        char display_name[256];
        snprintf(display_name, sizeof(display_name), "%s", active_station->name);
        sanitize_str(display_name);
        while (visual_width(display_name) > max_text_width) {
             display_name[strlen(display_name)-1] = 0;
             if (visual_width(display_name) <= max_text_width - 3) { strcat(display_name, "..."); break; }
        }
        int pad_name = max_text_width - visual_width(display_name);
        if (pad_name < 0) pad_name = 0;

        // Top Border
        printf("\033[K\n\033[K%s╔", BORDER_COLOR);
        for(int i=0; i<content_width; i++) printf("═");
        printf("╗%s\n", COLOR_RESET);

        // Name Line
        printf("\033[K%s║%s ♫  " COLOR_BOLD_CYAN "NOW PLAYING:" COLOR_BOLD_WHITE " %s%*s %s║%s\n", BORDER_COLOR, COLOR_RESET, display_name, pad_name, "", BORDER_COLOR, COLOR_RESET);
        
        if (song_title && *song_title) {
            char display_title[256];
            snprintf(display_title, sizeof(display_title), "%s", song_title);
            sanitize_str(display_title);
            while (visual_width(display_title) > max_text_width) {
                 display_title[strlen(display_title)-1] = 0;
                 if (visual_width(display_title) <= max_text_width - 3) { strcat(display_title, "..."); break; }
            }
            int pad_title = max_text_width - visual_width(display_title);
            if (pad_title < 0) pad_title = 0;
            
            // Align "INFO:" to match "NOW PLAYING:" length (12 chars) -> "INFO:       "
            printf("\033[K%s║%s ♬  " COLOR_CYAN "INFO:       " COLOR_WHITE " %s%*s %s║%s\n", BORDER_COLOR, COLOR_RESET, display_title, pad_title, "", BORDER_COLOR, COLOR_RESET);
        }
        
        // Bottom Border
        printf("\033[K%s╚", BORDER_COLOR);
        for(int i=0; i<content_width; i++) printf("═");
        printf("╝%s\n", COLOR_RESET);
    }

    // Clear the rest of the screen and flush
    printf("\033[J");
    fflush(stdout);
}
