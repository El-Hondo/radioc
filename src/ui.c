#include <stdio.h>
#include "ui.h"

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
    printf("\n--- %s ---\n", title);
    if (count == 0) {
        printf("No accessible stations found.\n");
        return;
    }
    for (int i = 0; i < count; i++) {
        printf("[%d] %s (%s) [%s]\n", i + 1, stations[i].name, stations[i].country, stations[i].tags);
    }
    printf("---------------------------\n");
}
