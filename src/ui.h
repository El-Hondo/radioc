#ifndef UI_H
#define UI_H

#include "api.h"

void ui_print_help();
void ui_print_credits();
void ui_print_stations(const Station *stations, int count, const char *title);

#endif
