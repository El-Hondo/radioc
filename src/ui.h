#ifndef UI_H
#define UI_H

#include "api.h"

void ui_print_help();
void ui_print_credits();
void ui_print_stations(const Station *stations, int count, const char *title);


// Initialize UI (e.g. enter alternate screen buffer)
void ui_init();

// ANSI Color Codes
#define COLOR_RESET       "\033[0m"
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

#define BORDER_COLOR      COLOR_BLUE
#define HEADER_COLOR      COLOR_BOLD_CYAN
#define ID_COLOR          COLOR_YELLOW
#define NAME_COLOR        COLOR_BOLD_WHITE
#define META_COLOR        COLOR_CYAN
#define TAGS_COLOR        COLOR_MAGENTA
#define VOTES_COLOR       COLOR_GREEN

// Cleanup UI (e.g. leave alternate screen buffer)
void ui_cleanup();

// Clears the terminal screen
void ui_clear_screen();

// Main render function to draw the persistent UI
void ui_render_interface(const Station *active_station, 
                         const char *song_title, 
                         const char *status_message, 
                         const Station *list, 
                         int list_count, 
                         const char *list_title);

#endif
