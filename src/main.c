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
#include "tui_input.h"
#include "config.h"

// State to track what the indices [1], [2] refer to
// It can be either the search results or the favorites list
// State to track what the indices [1], [2] refer to
// It can be either the search results or the favorites list
typedef enum {
    MODE_FAVORITES,
    MODE_SEARCH_RESULTS
} list_mode_t;

// State to track the active view
typedef enum {
    VIEW_MAIN,
    VIEW_HELP,
    VIEW_CREDITS
} view_mode_t;

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

    // UI State
    Station *active_station = NULL;
    char *current_song_title = NULL; // Helper to track title
    char status_message[256] = "";
    int show_list = 1; // Start by showing favorites

    // Load Config
    AppConfig config;
    if (config_load(&config) != 0) {
        snprintf(status_message, sizeof(status_message), "Loaded default config.");
    } else {
        snprintf(status_message, sizeof(status_message), "Loaded config.");
    }
    
    // Apply Settings
    ui_set_theme(config.theme); // Apply loaded theme
    player_set_volume(config.volume - player_get_volume());
    // Wait, player_set_volume adds/subtracts. We need absolute set.
    // Let's assume player starts at ~50 or we should fix player API?
    // Let's fix player API in next step. For now, hack it:
    // Actually, libvlc defaults to 100 often, or 50.
    // Best way: player_set_volume currently is relative. 
    // I entered a relative implementation in player.c. I should add player_set_volume_absolute.
    // Or just loop until it matches? No.
    // Let's modify player.c to add absolute set, or direct access.
    // I'll stick to logic: current vol = X. Target = config.volume. Adjustment = Target - X.
    // player_set_volume(config.volume - player_get_volume()) works nicely if player_get_volume works!
    int current_vol = player_get_volume();
    player_set_volume(config.volume - current_vol);


    // State
    Station search_results[MAX_STATIONS];
    int search_count = 0;
    
    // Pointers for current list context
    list_mode_t current_mode = MODE_FAVORITES;
    view_mode_t current_view = VIEW_MAIN;
    
    int fav_count = 0;
    // Load initial favorites
    Station* fav_list = favorites_get_list(&fav_count);



    // Initial render
    ui_render_interface(active_station, NULL, "Welcome to VibeRadio!", fav_list, fav_count, "Favorites", 0);

    // Main Loop
    int running = 1;

    // --- TUI Mode ---
    tui_enable_raw_mode();
    
    // Clear screen initially
    ui_render_interface(active_station, NULL, "TUI Mode Enabled. Press 'q' to quit.", fav_list, fav_count, "Favorites", 0);

    // Selected index for navigation
    int selected_global_index = 0; // 0-based index in the current list
    
    while (running) {
        // Read key (with timeout)
        int c = tui_read_key();
        
        if (c == 0) {
            // No input, but we might want to update UI if song changed?
            // For now, doing nothing saves CPU.
            // Check metadata periodically?
            // This loop runs every 100ms approx due to VTIME=1
            continue; 
        }

        char status_buf[256] = "";

        // Process Key
        switch (c) {
            case 'q':
                // Save Config on Exit
                config.volume = player_get_volume();
                config_save(&config);
                running = 0;
                break;
                
            case 's':
                player_stop();
                active_station = NULL;
                if (current_song_title) { free(current_song_title); current_song_title = NULL; }
                snprintf(status_buf, sizeof(status_buf), "Playback stopped.");
                break;
                
            case ' ': // Space to Pause/Resume? LibVLC toggle pause?
                // For now, let's make it STOP to match 's' or maybe just Play selected?
                // Let's make Space = Stop for safety for now.
                player_stop();
                snprintf(status_buf, sizeof(status_buf), "Stopped.");
                break;

            case ARROW_UP:
                if (current_view == VIEW_MAIN && selected_global_index > 0) selected_global_index--;
                break;
                
            case ARROW_DOWN:
                 // Count items
                 if (current_view == VIEW_MAIN) {
                    int max_items = (current_mode == MODE_FAVORITES) ? fav_count : search_count;
                    if (selected_global_index < max_items - 1) selected_global_index++;
                 }
                break;

            case PAGE_UP:
            case ARROW_LEFT: // Option for left/right paging
                if (current_view == VIEW_MAIN) {
                    if (selected_global_index >= 25) {
                        selected_global_index -= 25;
                    } else {
                        selected_global_index = 0;
                    }
                }
                break;

            case PAGE_DOWN:
            case ARROW_RIGHT:
                if (current_view == VIEW_MAIN) {
                    int max_items = (current_mode == MODE_FAVORITES) ? fav_count : search_count;
                    if (selected_global_index + 25 < max_items) {
                        selected_global_index += 25;
                    } else {
                        selected_global_index = max_items - 1;
                    }
                }
                break;

            case '\r': // Enter
            case '\n':
                if (current_view == VIEW_MAIN) {
                    Station *target_list = (current_mode == MODE_SEARCH_RESULTS) ? search_results : fav_list;
                    int max_len = (current_mode == MODE_SEARCH_RESULTS) ? search_count : fav_count;
                    
                    if (selected_global_index >= 0 && selected_global_index < max_len) {
                         Station *s = &target_list[selected_global_index];
                         
                         // Create static copy for active station
                         static Station active_station_store;
                         active_station_store = *s;
                         active_station = &active_station_store;

                         if (player_play(s->url) != 0) {
                             snprintf(status_buf, sizeof(status_buf), "Error starting playback.");
                             active_station = NULL;
                         } else {
                             snprintf(status_buf, sizeof(status_buf), "Playing: %s", s->name);
                             // Clear old song title so it doesn't persist
                             if (current_song_title) {
                                 free(current_song_title);
                                 current_song_title = NULL;
                             }
                         }
                    }
                }
                break;
                
            case 'l': // List Favorites
                 fav_list = favorites_get_list(&fav_count);
                 current_mode = MODE_FAVORITES;
                 current_view = VIEW_MAIN; // Return to main view
                 selected_global_index = 0;
                 show_list = 1;
                 snprintf(status_buf, sizeof(status_buf), "Switched to Favorites.");
                 break;

            case 'h':
            case '?':
                if (current_view == VIEW_HELP) current_view = VIEW_MAIN;
                else current_view = VIEW_HELP;
                break;

            case 'C': // Shift+C usually
                if (current_view == VIEW_CREDITS) current_view = VIEW_MAIN;
                else current_view = VIEW_CREDITS;
                break;

            case '+':
            case '=': // Support unshifted + too?
                {
                   int v = player_set_volume(5);
                   snprintf(status_buf, sizeof(status_buf), "Volume: %d%%", v);
                }
                break;
                
            case '-':
            case '_':
               {
                   int v = player_set_volume(-5);
                   snprintf(status_buf, sizeof(status_buf), "Volume: %d%%", v);
               }
               break;

            case 'f': // Find
                {
                    // 1. Temporarily disable Raw Mode
                    tui_disable_raw_mode();
                    
                    // 2. Clear screen and Prompt
                    ui_clear_screen();
                    printf(COLOR_BOLD_CYAN "Search for station: %s", COLOR_RESET);
                    fflush(stdout);
                    
                    // 3. Read Input (Canonically!)
                    char search_buf[256];
                    if (fgets(search_buf, sizeof(search_buf), stdin)) {
                        // Strip newline
                        size_t len = strlen(search_buf);
                        if (len > 0 && search_buf[len-1] == '\n') search_buf[len-1] = '\0';
                        
                        // 4. Run Search
                        if (strlen(search_buf) > 0) {
                            printf("\n" COLOR_CYAN "Searching...%s", COLOR_RESET);
                            int n = api_search_stations(search_buf, search_results, MAX_STATIONS);
                            if (n >= 0) {
                                search_count = n;
                                current_mode = MODE_SEARCH_RESULTS;
                                current_view = VIEW_MAIN; // Go to results
                                show_list = 1;
                                selected_global_index = 0; // Reset selection
                                snprintf(status_buf, sizeof(status_buf), "Found %d stations for '%s'.", n, search_buf);
                            } else {
                                snprintf(status_buf, sizeof(status_buf), "Search failed.");
                            }
                        } else {
                             snprintf(status_buf, sizeof(status_buf), "Search cancelled.");
                        }
                    }
                    
                    // 5. Re-enable Raw Mode
                    tui_enable_raw_mode();
                }
                break;
            
            case 't': // Tag Search
                {
                    tui_disable_raw_mode();
                    ui_clear_screen();
                    printf(COLOR_BOLD_MAGENTA "Search by Tag: %s", COLOR_RESET);
                    fflush(stdout);
                    
                    char search_buf[256];
                    if (fgets(search_buf, sizeof(search_buf), stdin)) {
                        size_t len = strlen(search_buf);
                        if (len > 0 && search_buf[len-1] == '\n') search_buf[len-1] = '\0';
                        
                        if (strlen(search_buf) > 0) {
                            printf("\n" COLOR_MAGENTA "Searching tags...%s", COLOR_RESET);
                            int n = api_search_by_tag(search_buf, search_results, MAX_STATIONS);
                            if (n >= 0) {
                                search_count = n;
                                current_mode = MODE_SEARCH_RESULTS;
                                current_view = VIEW_MAIN;
                                show_list = 1;
                                selected_global_index = 0;
                                snprintf(status_buf, sizeof(status_buf), "Found %d stations with tag '%s'.", n, search_buf);
                            } else {
                                snprintf(status_buf, sizeof(status_buf), "Search failed.");
                            }
                        } else {
                             snprintf(status_buf, sizeof(status_buf), "Search cancelled.");
                        }
                    }
                    tui_enable_raw_mode();
                }
                break;
            
            case 'T': // Cycle Theme
                ui_cycle_theme();
                // Update config struct immediately?
                strncpy(config.theme, current_theme.name, sizeof(config.theme));
                snprintf(status_buf, sizeof(status_buf), "Theme: %s", current_theme.name);
                break;

            case 'c': // Country Search
                {
                    tui_disable_raw_mode();
                    ui_clear_screen();
                    printf(COLOR_BOLD_YELLOW "Search by Country Code (e.g. DE, US): %s", COLOR_RESET);
                    fflush(stdout);
                    
                    char search_buf[256];
                    if (fgets(search_buf, sizeof(search_buf), stdin)) {
                        size_t len = strlen(search_buf);
                        if (len > 0 && search_buf[len-1] == '\n') search_buf[len-1] = '\0';
                        
                        if (strlen(search_buf) > 0) {
                            printf("\n" COLOR_YELLOW "Searching countries...%s", COLOR_RESET);
                            int n = api_search_by_country(search_buf, search_results, MAX_STATIONS);
                            if (n >= 0) {
                                search_count = n;
                                current_mode = MODE_SEARCH_RESULTS;
                                current_view = VIEW_MAIN;
                                show_list = 1;
                                selected_global_index = 0;
                                snprintf(status_buf, sizeof(status_buf), "Found %d stations in '%s'.", n, search_buf);
                            } else {
                                snprintf(status_buf, sizeof(status_buf), "Search failed.");
                            }
                        } else {
                             snprintf(status_buf, sizeof(status_buf), "Search cancelled.");
                        }
                    }
                    tui_enable_raw_mode();
                }
                break;

            default:
                // snprintf(status_buf, sizeof(status_buf), "Key: %d", c); // Debug
                break;
        }


        // --- Render Phase ---
        
        // 1. Fetch Metadata if playing (simple check)
        if (active_station) {
            char *meta = player_get_metadata();
            if (active_station && meta) {
                 if (current_song_title) free(current_song_title);
                 current_song_title = meta;
            } else if (!meta && current_song_title) {
                // Keep old title? Or clear?
            }
        }

        ui_clear_screen();
        if (current_view == VIEW_HELP) {
            ui_print_help();
            if (active_station) {
               // Optional: Show mini player status at bottom?
               // For now, simpler is better.
            }
        } else if (current_view == VIEW_CREDITS) {
            ui_print_credits();
        } else {
             // VIEW_MAIN
             Station *list_to_show = NULL;
             int list_count_to_show = 0;
             const char *list_title = NULL;

             if (current_mode == MODE_FAVORITES) {
                 list_to_show = fav_list;
                 list_count_to_show = fav_count;
                 list_title = "Favorites";
             } else {
                 list_to_show = search_results;
                 list_count_to_show = search_count;
                 list_title = "Search Results";
             }
             
             ui_render_interface(active_station, current_song_title, status_buf, list_to_show, list_count_to_show, list_title, selected_global_index);
        }

        fflush(stdout);
    } // End While

    // Cleanup
    if (current_song_title) free(current_song_title);
    player_cleanup();
    
    // Restore main screen buffer
    ui_cleanup();
    tui_disable_raw_mode();
    
    return 0;
}
