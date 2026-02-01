/**
 * @file tui_input.c
 * @brief Raw terminal input handling.
 */
#include <ctype.h>
#include <errno.h>
#include <stdio.h>
#include <stdlib.h>
#include <termios.h>
#include <unistd.h>
#include "tui_input.h"

static struct termios orig_termios;

void tui_disable_raw_mode() {
  if (tcsetattr(STDIN_FILENO, TCSAFLUSH, &orig_termios) == -1)
    perror("tcsetattr");
}

void tui_enable_raw_mode() {
  if (tcgetattr(STDIN_FILENO, &orig_termios) == -1) {
      perror("tcgetattr");
      return;
  }
  
  // Ensure we disable raw mode at exit
  atexit(tui_disable_raw_mode);

  struct termios raw = orig_termios;
  
  // Input flags: disable flow control, etc.
  raw.c_iflag &= ~(BRKINT | ICRNL | INPCK | ISTRIP | IXON);
  
  // Output flags:
  // We keep OPOST enabled so that \n is automatically translated to \r\n.
  // If we disabled OPOST, we would need to fix all printfs to use \r\n manually. 

  // Control flags
  raw.c_cflag |= (CS8);

  // Local flags: disable echo, canonical mode (line buffering), signals (ctrl-c/z)
  raw.c_lflag &= ~(ECHO | ICANON | IEXTEN | ISIG);

  // Timeout for read: 100ms timeout (VTIME=1), return as soon as data available (VMIN=0)
  raw.c_cc[VMIN] = 0;
  raw.c_cc[VTIME] = 1; 

  if (tcsetattr(STDIN_FILENO, TCSAFLUSH, &raw) == -1) 
    perror("tcsetattr");
}

int tui_read_key() {
  int nread;
  char c;
  while ((nread = read(STDIN_FILENO, &c, 1)) != 1) {
    if (nread == -1 && errno != EAGAIN) return -1;
    // If VTIME expires (0 read), we return 0
    if (nread == 0) return 0;
  }

  // Handle Escape Sequences (Arrows, etc)
  if (c == '\x1b') {
    char seq[3];
    if (read(STDIN_FILENO, &seq[0], 1) != 1) return '\x1b';
    if (read(STDIN_FILENO, &seq[1], 1) != 1) return '\x1b';

    if (seq[0] == '[') {
      if (seq[1] >= '0' && seq[1] <= '9') {
        if (read(STDIN_FILENO, &seq[2], 1) != 1) return '\x1b';
        if (seq[2] == '~') {
          switch (seq[1]) {
            case '1': return HOME_KEY;
            case '3': return DEL_KEY;
            case '4': return END_KEY;
            case '5': return PAGE_UP;
            case '6': return PAGE_DOWN;
            case '7': return HOME_KEY;
            case '8': return END_KEY;
          }
        }
      } else {
        switch (seq[1]) {
          case 'A': return ARROW_UP;
          case 'B': return ARROW_DOWN;
          case 'C': return ARROW_RIGHT;
          case 'D': return ARROW_LEFT;
          case 'H': return HOME_KEY;
          case 'F': return END_KEY;
        }
      }
    }
    return '\x1b';
  }
  return c;
}
