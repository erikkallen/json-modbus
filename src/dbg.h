#ifndef DBG_H
#define DBG_H

#define E_COLOR_RED       "\033[0;31m"
#define E_COLOR_GREEN     "\033[0;32m"
#define E_COLOR_BLUE      "\033[0;34m"
#define E_COLOR_CYAN      "\033[0;36m"
#define E_COLOR_YELLOW    "\033[0;33m"
#define E_COLOR_WHITE     "\033[0;37m"
#define E_COLOR_BLACK     "\033[0;30m"
#define E_RESET     "\033[m"

#define E_COLOR_B_RED       "\033[1;31m"
#define E_COLOR_B_GREEN     "\033[1;32m"
#define E_COLOR_B_BLUE      "\033[1;34m"
#define E_COLOR_B_CYAN      "\033[1;36m"
#define E_COLOR_B_YELLOW    "\033[1;33m"
#define E_COLOR_B_WHITE     "\033[1;37m"
#define E_COLOR_B_BLACK     "\033[1;30m"

#define DEBUG_TAG(txt) (isatty(fileno(stderr)) ? E_COLOR_YELLOW "DEBUG: " E_RESET txt : "DEBUG: " txt)

#define DEBUG_MSG(msg,...) if (debug_mode) fprintf(stderr, DEBUG_TAG(msg), ##__VA_ARGS__);

#endif