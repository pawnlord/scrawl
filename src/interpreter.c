#include <stdio.h>
#include <stdlib.h>
#include <string.h>
#include "../inc/parse.h"
#include "../inc/handlers.h"
#define MAX_LINE_LENGTH 100

char* prompt = "scripts>";

volatile int* exit_loop;

void clear_str(char* str, int size){
	for(int i = 0; i < size; i++){
		str[i] = 0;
	}
}


int main(int argc, char** argv){
	/* check what mode they want (CLI or Interpreter) */
	int mode = argc > 1;
	
	int exit_loop_addr = 0;
	
	exit_loop = &exit_loop_addr;

	/* setup handler for ctrl^c 								   *	
	 * exit_loop: pointer to whether or not we should stop the loop*/
	setup_handler(exit_loop);

	if(mode == 0){
		/* input line */
		char* line = malloc(MAX_LINE_LENGTH);
		start_parser();

		/* introduction */
		printf("This is the Simple Crappy Resource Intensive, Poorly-made Train-wreck of a Script (scripts)\nversion 1.0.0 - type help for general commands\n");
		
		/* main loop */
		while(strcmp(line, "exit\n") && !(*exit_loop)){
			/* setup line */
			clear_str(line, MAX_LINE_LENGTH);

			printf("%s", prompt);
			/* get line to parse */
			fgets(line, MAX_LINE_LENGTH, stdin);
			
			/* return value */
			variable rv;
			parse(line, &rv, 0);
			
			/* output based on type */
			if(rv.t == INT8_e || rv.t == INT16_e || rv.t == INT32_e || rv.t == INT64_e){
				printf("%d\n", rv.value);
			}
		}
		free(line);
		stop_parser();
	}
}