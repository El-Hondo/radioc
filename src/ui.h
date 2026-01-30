#ifndef UI_H
#define UI_H

#include "api.h"

void ui_print_help();
void ui_print_credits();
void ui_print_stations(const Station *stations, int count, const char *title);


// Initialize UI (e.g. enter alternate screen buffer)
void ui_init();

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
