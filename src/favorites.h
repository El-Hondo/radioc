#ifndef FAVORITES_H
#define FAVORITES_H

#include "api.h"

#define MAX_FAVORITES 100

// Initialize/Load favorites from disk
// Returns number of favorites loaded
int favorites_load();

// Add a station to favorites
void favorites_add(const Station *s);

// Remove a station from favorites by index (0-based from list)
void favorites_remove(int index);

// Get the current list of favorites
Station* favorites_get_list(int *count);

// Save favorites to disk
void favorites_save();

#endif
