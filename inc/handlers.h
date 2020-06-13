/* This file is meant for handlers and functions related to handlers */

#if defined(WIN32) || defined(_WIN32) || defined(__WIN32) && !defined(__CYGWIN__)

#include <windows.h> 

/* we're windows, we have to go through the windows API*/
BOOL WINAPI handler(DWORD ctrlType);

#else
/* we're *nix most likely, use signals*/

#include <unistd>
/* TODO: Implement on *nix */
int handler(int garbage);

#endif

/* setup handlers depending on device */
int setup_handler(volatile int* state_variable_);
