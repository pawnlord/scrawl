#ifndef FUNCTIONS_H
#define FUNCTIONS_H
#define MAX_LINE_LENGTH 255
#include "data_information.h"
/* used in operator functions*/
int parse_tokens(token* tokens, variable* return_value, int line_num);

/* setup for parser */
int   start_parser(volatile int* exit_loop);

/* actual parse function */
int  parse(char* line, variable* return_value, int line_num, int is_newline);

/* "stack" simulation */
int   push_back_master_state();

/* stop parser, clean memory */
int   stop_parser();
#endif