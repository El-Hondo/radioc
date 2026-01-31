#ifndef UI_H
#define UI_H

#include "api.h"

void ui_print_help();
void ui_print_credits();


// Initialize UI (e.g. enter alternate screen buffer)
void ui_init();

// ANSI Color Codes
#define COLOR_RESET       current_theme.reset
#define COLOR_BOLD        "\033[1m"
#define COLOR_DIM         "\033[2m"
#define COLOR_ITALIC      "\033[3m"
#define COLOR_RED         "\033[31m"
#define COLOR_GREEN       "\033[32m"
#define COLOR_YELLOW      "\033[33m"
#define COLOR_BLUE        "\033[34m"
#define COLOR_MAGENTA     "\033[35m"
#define COLOR_CYAN        "\033[36m"
#define COLOR_WHITE       "\033[37m"
#define COLOR_BOLD_RED    "\033[1;31m"
#define COLOR_BOLD_GREEN  "\033[1;32m"
#define COLOR_BOLD_YELLOW "\033[1;33m"
#define COLOR_BOLD_BLUE   "\033[1;34m"
#define COLOR_BOLD_MAGENTA "\033[1;35m"
#define COLOR_BOLD_CYAN   "\033[1;36m"
#define COLOR_BOLD_WHITE  "\033[1;37m"

// Theme Structure
typedef struct {
    char name[32];
    const char *border;
    const char *header;
    const char *id;
    const char *name_col;
    const char *meta;
    const char *tags;
    const char *votes;
    const char *highlight_bg; // Background for selection
    const char *highlight_fg; // Foreground for selection
    const char *background;   // Global background color
    const char *reset;        // Custom reset string (e.g. \033[0m or \033[0m\033[44m)
} UITheme;

// Global current theme
extern UITheme current_theme;

// Color Macros (Now Dynamic)
#define BORDER_COLOR      current_theme.border
#define HEADER_COLOR      current_theme.header
#define ID_COLOR          current_theme.id
#define NAME_COLOR        current_theme.name_col
#define META_COLOR        current_theme.meta
#define TAGS_COLOR        current_theme.tags
#define VOTES_COLOR       current_theme.votes

// Function to set theme by name
void ui_set_theme(const char *theme_name);

// Cycle to next available theme
void ui_cycle_theme();

// Cleanup UI (e.g. leave alternate screen buffer)
void ui_cleanup();

// Clears the terminal screen
void ui_clear_screen();

// Main render function to draw the persistent UI
// selected_index: -1 for none, or 0-based index to highlight
void ui_render_interface(const Station *active_station, 
                         const char *song_title, 
                         const char *status_message, 
                         const Station *list, 
                         int list_count, 
                         const char *list_title,
                         int selected_index);

void ui_print_stations(const Station *stations, int count, const char *title, int selected_index, int start_index);

#endif
