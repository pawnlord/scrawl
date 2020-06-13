#include "../inc/handlers.h"
#include <stdio.h>

volatile int* state_variable;

#if defined(WIN32) || defined(_WIN32) || defined(__WIN32) && !defined(__CYGWIN__)

#include <windows.h> 
BOOL WINAPI handler(DWORD ctrlType){
    switch (ctrlType)
    {
        // Handle the CTRL-C signal. 
        case CTRL_C_EVENT:
            *state_variable = 1;
            return TRUE;
        default:
            return FALSE;
    }
}

#else

#include <unistd>
/* TODO: Implement on *nix */
int handler(int garbage);

#endif


int setup_handler(volatile int* state_variable_){
    state_variable = state_variable_;
#if defined(WIN32) || defined(_WIN32) || defined(__WIN32) && !defined(__CYGWIN__)

    if(!SetConsoleCtrlHandler(handler, TRUE)){
        /* handler went wrong. error */
        perror("scripts: setup_handler");
    }

#else
/* we're *nix probably */
/* TODO: Implement on *nix */
#endif
}