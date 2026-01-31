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

// Enable raw text mode (disables buffering and echoing)
void tui_enable_raw_mode();

// Restore terminal to normal canonical mode
void tui_disable_raw_mode();

// Read a single keypress. Returns the key code or 0 if no input waiting (if non-blocking)
// But standard read() blocks. We might want a blocking read that returns keys.
int tui_read_key();

#endif
