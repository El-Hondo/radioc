#include <stdio.h>
#include <string.h>
#include <stdlib.h>
#include <locale.h>
#include "api.h"
#include "player.h"
#include "favorites.h"
#include "ui.h"

// State to track what the indices [1], [2] refer to
// It can be either the search results or the favorites list
typedef enum {
    MODE_FAVORITES,
    MODE_SEARCH_RESULTS
} list_mode_t;

int main() {
    setlocale(LC_ALL, ""); // Enable system locale for wide char support
    setlocale(LC_NUMERIC, "C"); // Keep numeric locale "C" for correct parsing of numbers (JSON etc)

    // Initialization
    if (player_init() != 0) {
        fprintf(stderr, "Failed to initialize player.\n");
        return 1;
    }
    favorites_load();

    // State
    Station search_results[MAX_STATIONS];
    int search_count = 0;
    
    // Pointers for current list context
    // Default to displaying favorites
    list_mode_t current_mode = MODE_FAVORITES;
    
    int fav_count = 0;
    Station* fav_list = favorites_get_list(&fav_count);
    ui_print_stations(fav_list, fav_count, "Favorites");

    // Main Loop
    char line[256];
    int running = 1;

    while (running) {
        printf("\nviberadio> ");
        if (!fgets(line, sizeof(line), stdin)) break; // EOF

        // Strip newline
        line[strcspn(line, "\n")] = 0;
        if (strlen(line) == 0) continue;

        char cmd[32];
        char arg[224];
        
        // Simple parser
        int args = sscanf(line, "%31s %[^\n]", cmd, arg); // Read cmd and potentially rest of line

        if (strcmp(cmd, "quit") == 0 || strcmp(cmd, "q") == 0) {
            running = 0;
        } 
        else if (strcmp(cmd, "help") == 0 || strcmp(cmd, "?") == 0) {
            ui_print_help();
        }
        else if (strcmp(cmd, "stop") == 0 || strcmp(cmd, "s") == 0) {
            player_stop();
            printf("Stopped.\n");
        }
        else if (strcmp(cmd, "list") == 0 || strcmp(cmd, "l") == 0 ) {
            fav_list = favorites_get_list(&fav_count);
            ui_print_stations(fav_list, fav_count, "Favorites");
            current_mode = MODE_FAVORITES;
        }
        else if (strcmp(cmd, "find") == 0 || strcmp(cmd, "f") == 0) {
            if (args < 2) {
                printf("Usage: find <term>\n");
            } else {
                printf("Finding stations for '%s'...\n", arg);
                int n = api_search_stations(arg, search_results, MAX_STATIONS);
                if (n >= 0) {
                    search_count = n;
                    ui_print_stations(search_results, search_count, "Found Stations");
                    current_mode = MODE_SEARCH_RESULTS;
                } else {
                    printf("Search failed.\n");
                }
            }
        }
        else if (strcmp(cmd, "tag") == 0 || strcmp(cmd, "t") == 0) {
            if (args < 2) {
                printf("Usage: tag <term> (e.g. jazz, news)\n");
            } else {
                printf("Searching for stations with tag '%s'...\n", arg);
                int n = api_search_by_tag(arg, search_results, MAX_STATIONS);
                if (n >= 0) {
                    search_count = n;
                    ui_print_stations(search_results, search_count, "Tag Search Results");
                    current_mode = MODE_SEARCH_RESULTS;
                } else {
                    printf("Search failed.\n");
                }
            }
        }
        else if (strcmp(cmd, "country") == 0 || strcmp(cmd, "c") == 0) {
            if (args < 2) {
                printf("Usage: country <code> (e.g. US, DE)\n");
            } else {
                printf("Searching for stations in '%s'...\n", arg);
                int n = api_search_by_country(arg, search_results, MAX_STATIONS);
                if (n >= 0) {
                    search_count = n;
                    ui_print_stations(search_results, search_count, "Country Search Results");
                    current_mode = MODE_SEARCH_RESULTS;
                } else {
                    printf("Search failed.\n");
                }
            }
        }
        else if (strcmp(cmd, "play") == 0 || strcmp(cmd, "p") == 0) {
            if (args < 2) {
                printf("Usage: play <id>\n");
            } else {
                int id = atoi(arg);
                Station *target_list = (current_mode == MODE_SEARCH_RESULTS) ? search_results : favorites_get_list(&fav_count);
                int max_len = (current_mode == MODE_SEARCH_RESULTS) ? search_count : fav_count;

                if (id > 0 && id <= max_len) {
                    Station *s = &target_list[id-1];
                    printf("Playing: %s (%s)\n", s->name, s->url);
                    if (player_play(s->url) != 0) {
                        printf("Error starting playback.\n");
                    }
                } else {
                    printf("Invalid ID.\n");
                }
            }
        }
        else if (strcmp(cmd, "add") == 0 || strcmp(cmd, "a") == 0) {
             if (args < 2) {
                printf("Usage: add <id> (from current list)\n");
            } else {
                int id = atoi(arg);
                Station *target_list = (current_mode == MODE_SEARCH_RESULTS) ? search_results : favorites_get_list(&fav_count);
                int max_len = (current_mode == MODE_SEARCH_RESULTS) ? search_count : fav_count;

                if (id > 0 && id <= max_len) {
                    favorites_add(&target_list[id-1]);
                    printf("Added '%s' to favorites.\n", target_list[id-1].name);
                } else {
                    printf("Invalid ID.\n");
                }
            }
        }
        else if (strcmp(cmd, "del") == 0 || strcmp(cmd, "d") == 0) {
             if (args < 2) {
                printf("Usage: del <id> (from favorites list)\n");
            } else {
                // Deletion makes most sense from the favorites list view
                // Attempting to delete from search results is ambiguous (delete from where?)
                // So we enforce explicit context or parameter. 
                // However, to keep it simple, we will map 'id' to the current favorites list IF we are in favorites mode.
                // If in search mode, user should probably switch to list first, or we can just say "Switch to list to delete".
                
                if (current_mode != MODE_FAVORITES) {
                    printf("Please switch to favorites list first ('list') to delete items.\n");
                } else {
                     int id = atoi(arg);
                     if (id > 0 && id <= fav_count) {
                        printf("Removing '%s' from favorites.\n", fav_list[id-1].name);
                        favorites_remove(id-1);
                        fav_list = favorites_get_list(&fav_count); // refresh
                        ui_print_stations(fav_list, fav_count, "Favorites");
                     } else {
                         printf("Invalid ID.\n");
                     }
                }
            }
        }
        else {
            printf("Unknown command. Type 'help' for options.\n");
        }
    }

    player_cleanup();
    return 0;
}
