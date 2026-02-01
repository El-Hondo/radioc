#ifndef TUI_INPUT_H
#define TUI_INPUT_H

// Key definitions for special keys
enum EditorKey {
  BACKSPACE = 127,
  ARROW_LEFT = 1000,
  ARROW_RIGHT,
  ARROW_UP,
  ARROW_DOWN,
  DEL_KEY,
  HOME_KEY,
  END_KEY,
  PAGE_UP,
  PAGE_DOWN
};

/**
 * @brief Enable raw text mode.
 * Disables canonical mode (line buffering) and echoing to read keys directly.
 */
void tui_enable_raw_mode();

/**
 * @brief Restore terminal to normal canonical mode.
 */
void tui_disable_raw_mode();

/**
 * @brief Read a single keypress.
 * @return The key code (char or special enum value) or 0 if no input is waiting.
 */
int tui_read_key();

#endif
