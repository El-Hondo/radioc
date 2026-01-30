#include <stdio.h>
#include <string.h>
#include <stdlib.h>
#include <locale.h>
#include "api.h"
#include "player.h"
#include "favorites.h"
#include "ui.h"
#include <readline/readline.h>
#include <readline/history.h>

// State to track what the indices [1], [2] refer to
// It can be either the search results or the favorites list
typedef enum {
    MODE_FAVORITES,
    MODE_SEARCH_RESULTS
} list_mode_t;

int main() {
    setlocale(LC_ALL, ""); // Enable system locale for wide char support
    setlocale(LC_NUMERIC, "C"); // Keep numeric locale "C" for correct parsing of numbers (JSON etc)
    
    // Switch to alternate screen buffer
    ui_init();

    // Initialization
    if (player_init() != 0) {
        fprintf(stderr, "Failed to initialize player.\n");
        return 1;
    }
    favorites_load();

    // State
    // State
    Station search_results[MAX_STATIONS];
    int search_count = 0;
    
    // Pointers for current list context
    list_mode_t current_mode = MODE_FAVORITES;
    
    int fav_count = 0;
    // Load initial favorites
    Station* fav_list = favorites_get_list(&fav_count);

    // UI State
    Station *active_station = NULL;
    char *current_song_title = NULL; // Helper to track title
    char status_message[256] = "";  
    int show_list = 1; // Start by showing favorites

    // Initial render
    ui_render_interface(active_station, NULL, "Welcome to VibeRadio!", fav_list, fav_count, "Favorites");

    // Main Loop
    char line[256];
    int running = 1;

    while (running) {
        char *input_buf = readline("\nviberadio> ");
        if (!input_buf) break; // EOF/Cntrl-D

        if (strlen(input_buf) > 0) {
            add_history(input_buf);
        }

        // Copy into our existing line buffer to minimalize refactor
        snprintf(line, sizeof(line), "%s", input_buf);
        free(input_buf);

        if (strlen(line) == 0) continue;

        char cmd[32];
        char arg[224];
        
        // Default state updates per loop
        // Default state updates per loop
        show_list = 1; // KEEP LIST VISIBLE per user request ("last list needs to stay visible")
        
        // "When an action without a new list is done ... only a message ... is written"
        // This was interpreted as "hide list", but user corrected: "needs to stay visible".
        // So we keep the list, but just update the status message.

        status_message[0] = '\0'; // Clear previous status
        
        // Simple parser
        arg[0] = '\0'; // Ensure arg is empty if not matched
        int args = sscanf(line, "%31s %[^\n]", cmd, arg); 

        if (strcmp(cmd, "quit") == 0 || strcmp(cmd, "q") == 0) {
            running = 0;
            continue;
        } 
        else if (strcmp(cmd, "help") == 0 || strcmp(cmd, "?") == 0) {
            // Help is a special case, we probably want to show it. 
            // We can just print it. ui_render_interface handles clearing, so we should call it first?
            // Or maybe help is just a list?
            // Let's print help directly after clear, or modify render interface?
            // For now, let's treat help as a "status" or just print it.
            // Since ui_render_interface clears screen, we should probably output help via a distinct way or just let it scroll if we didn't clear?
            // "if a list is created clear the screen before".
            // Let's rely on ui_render_interface to set the stage.
             snprintf(status_message, sizeof(status_message), "Help displayed above.");
             ui_clear_screen();
             ui_print_help();
             // active_station might be playing, so we might want to show it?
             // But help is big. Let's just pause the "persistent UI" for help? 
             // Or append help to status? 
             // Let's go with: Clear, Print Help, then Print "Now Playing" at bottom.
             // We can hack this by calling ui_render_interface with NULL list but print help manually before?
             // No, ui_render_interface clears screen at start.
             // Let's skip ui_render_interface for help command for now? 
             // "So if a list is created clear the screen before."
             // Help isn't a list.
        }
        else if (strcmp(cmd, "stop") == 0 || strcmp(cmd, "s") == 0) {
            player_stop();
            active_station = NULL;
            if (current_song_title) { free(current_song_title); current_song_title = NULL; }
            snprintf(status_message, sizeof(status_message), "Playback stopped.");
        }
        else if (strcmp(cmd, "list") == 0 || strcmp(cmd, "l") == 0 ) {
            fav_list = favorites_get_list(&fav_count);
            current_mode = MODE_FAVORITES;
            show_list = 1;
        }
        else if (strcmp(cmd, "find") == 0 || strcmp(cmd, "f") == 0) {
            if (args < 2 || strlen(arg) == 0) { // Check strlen arg because sscanf might fail to populate it
                 snprintf(status_message, sizeof(status_message), "Usage: find <term>");
            } else {
                printf("Finding..."); // Temporary feedback?
                int n = api_search_stations(arg, search_results, MAX_STATIONS);
                if (n >= 0) {
                    search_count = n;
                    current_mode = MODE_SEARCH_RESULTS;
                    show_list = 1;
                    snprintf(status_message, sizeof(status_message), "Found %d stations for '%s'.", n, arg);
                } else {
                    snprintf(status_message, sizeof(status_message), "Search failed.");
                }
            }
        }
        else if (strcmp(cmd, "tag") == 0 || strcmp(cmd, "t") == 0) {
             if (args < 2 || strlen(arg) == 0) {
                snprintf(status_message, sizeof(status_message), "Usage: tag <term>");
            } else {
                int n = api_search_by_tag(arg, search_results, MAX_STATIONS);
                if (n >= 0) {
                    search_count = n;
                    current_mode = MODE_SEARCH_RESULTS;
                    show_list = 1;
                    snprintf(status_message, sizeof(status_message), "Found %d stations with tag '%s'.", n, arg);
                } else {
                    snprintf(status_message, sizeof(status_message), "Search failed.");
                }
            }
        }
        else if (strcmp(cmd, "country") == 0 || strcmp(cmd, "c") == 0) {
             if (args < 2 || strlen(arg) == 0) {
                snprintf(status_message, sizeof(status_message), "Usage: country <code>");
            } else {
                int n = api_search_by_country(arg, search_results, MAX_STATIONS);
                if (n >= 0) {
                    search_count = n;
                    current_mode = MODE_SEARCH_RESULTS;
                    show_list = 1;
                    snprintf(status_message, sizeof(status_message), "Found %d stations in '%s'.", n, arg);
                } else {
                    snprintf(status_message, sizeof(status_message), "Search failed.");
                }
            }
        }
        else if (strcmp(cmd, "play") == 0 || strcmp(cmd, "p") == 0) {
            if (args < 2) {
                snprintf(status_message, sizeof(status_message), "Usage: play <id>");
            } else {
                int id = atoi(arg);
                Station *target_list = (current_mode == MODE_SEARCH_RESULTS) ? search_results : favorites_get_list(&fav_count);
                int max_len = (current_mode == MODE_SEARCH_RESULTS) ? search_count : fav_count;

                if (id > 0 && id <= max_len) {
                    Station *s = &target_list[id-1];
                    // Update Active Station
                    // We need to copy it because target_list might change if we search again? 
                    // No, search_results is static array, but content changes. 
                    // Ideally we should have a persistent copy of active station.
                    // For now, we point to it, but if user searches again, pointer is invalid?
                    // Actually yes, if search_results is overwritten.
                    // We should rely on a separate 'active_station_copy' or just risk it (it's a CLI).
                    // Let's just point to it for now, but be aware.
                    // Wait, if I play from search results, then 'find' something else, 'active_station' (pointer) will point to new garbage or new station.
                    // THIS IS A BUG. I need to copy the station.
                    // But Station struct is simple? Struct is: name, url, etc.
                    // Let's use a static Station instance for active_station storage.
                    static Station active_station_store;
                    active_station_store = *s;
                    active_station = &active_station_store;

                    if (player_play(s->url) != 0) {
                        snprintf(status_message, sizeof(status_message), "Error starting playback.");
                        active_station = NULL;
                    } else {
                        snprintf(status_message, sizeof(status_message), "Playing: %s", s->name);
                    }
                } else {
                    snprintf(status_message, sizeof(status_message), "Invalid ID.");
                }
            }
        }
        else if (strcmp(cmd, "add") == 0 || strcmp(cmd, "a") == 0) {
             if (args < 2) {
                snprintf(status_message, sizeof(status_message), "Usage: add <id>");
            } else {
                int id = atoi(arg);
                Station *target_list = (current_mode == MODE_SEARCH_RESULTS) ? search_results : favorites_get_list(&fav_count);
                int max_len = (current_mode == MODE_SEARCH_RESULTS) ? search_count : fav_count;

                if (id > 0 && id <= max_len) {
                    favorites_add(&target_list[id-1]);
                    snprintf(status_message, sizeof(status_message), "Added '%s' to favorites.", target_list[id-1].name);
                    // Refresh count just in case we are in fav mode
                    if (current_mode == MODE_FAVORITES) { 
                        fav_list = favorites_get_list(&fav_count);
                        show_list = 1; // Show updated list? "Whenever an action without a new list is done ... only a message".
                        // Add IS an action that could imply list update if we are viewing favorites.
                        // But strictly following user rule: "only a message ... is written".
                        // So I will set show_list = 0.
                        show_list = 0; 
                    }
                } else {
                    snprintf(status_message, sizeof(status_message), "Invalid ID.");
                }
            }
        }
        else if (strcmp(cmd, "del") == 0 || strcmp(cmd, "d") == 0) {
             if (args < 2) {
                snprintf(status_message, sizeof(status_message), "Usage: del <id> (from favorites)");
            } else {
                if (current_mode != MODE_FAVORITES) {
                    snprintf(status_message, sizeof(status_message), "Switch to favorites list first.");
                } else {
                     int id = atoi(arg);
                     if (id > 0 && id <= fav_count) {
                        snprintf(status_message, sizeof(status_message), "Removed '%s' from favorites.", fav_list[id-1].name);
                        favorites_remove(id-1);
                        fav_list = favorites_get_list(&fav_count);
                        // show_list = 1? Deleting an item and then showing blank screen with message might be confusing.
                        // But rule says "only a message ... is written".
                        show_list = 0;
                     } else {
                         snprintf(status_message, sizeof(status_message), "Invalid ID.");
                     }
                }
            }
        }
        else if (strcmp(cmd, "credits") == 0) {
             ui_clear_screen();
             ui_print_credits();
             // Treat as help (manual render)
             continue; // Skip main render
        }
        else {
            snprintf(status_message, sizeof(status_message), "Unknown command.");
        }

        // --- Render Phase ---
        
        // 1. Fetch Metadata if playing
        if (active_station) {
            char *meta = player_get_metadata();
            // If new meta is strictly different, update it.
            // Note: simple pointer replace if we free old one.
            if (current_song_title) free(current_song_title);
            current_song_title = meta; // Might be NULL
        }

        // 2. Determine List to Show
        Station *list_to_show = NULL;
        int count_to_show = 0;
        const char *title_to_show = NULL;

        if (show_list) {
            if (current_mode == MODE_FAVORITES) {
                // Ensure fresh
                fav_list = favorites_get_list(&fav_count);
                list_to_show = fav_list;
                count_to_show = fav_count;
                title_to_show = "Favorites";
            } else {
                list_to_show = search_results;
                count_to_show = search_count;
                title_to_show = "Search Results";
            }
        }

        // 3. Render
        if (strcmp(cmd, "help") != 0 && strcmp(cmd, "?") != 0) { // Don't wipe help immediately? 
            // Valid point: if I type help, I want to see it.
            // The loop will print prompt.
            // If I rendered help in the block above, and then I come here...
            // ui_render_interface clears the screen.
            // So if I typed help, I should skip `ui_render_interface`.
            // Added `continue` in help block? No I didn't.
            // Let's add specific handling.
            ui_render_interface(active_station, current_song_title, status_message, list_to_show, count_to_show, title_to_show);
        }
    }

    if (current_song_title) free(current_song_title);
    player_cleanup();
    
    // Restore main screen buffer
    ui_cleanup();
    
    return 0;
}
