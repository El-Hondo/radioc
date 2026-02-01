#ifndef FAVORITES_H
#define FAVORITES_H

#include "api.h"

#define MAX_FAVORITES 100

/**
 * @brief Initialize and load favorites from the standard location.
 * @return Number of favorites loaded.
 */
int favorites_load();

/**
 * @brief Add a station to the favorites list.
 * @param s Pointer to the Station struct to add.
 */
void favorites_add(const Station *s);

/**
 * @brief Remove a station from favorites by its index.
 * @param index The 0-based index of the station to remove.
 */
void favorites_remove(int index);

/**
 * @brief Get the current list of favorites.
 * @param count Output pointer to store the number of favorites.
 * @return Pointer to the internal favorites array.
 */
Station* favorites_get_list(int *count);

/**
 * @brief Save the current favorites list to disk.
 */
void favorites_save();

#endif
