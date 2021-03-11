#include <stdio.h>
#include <stdlib.h>
#include <string.h>
#include "../inc/parse.h"
#include "../inc/handlers.h"

char* prompt = "scrawl> ";

volatile int* exit_loop;

int readfile(FILE* fp, char** buff) {
	char c = 0;
	int i = 0;
	int l_num = 0;
	// repeat until c is EOF
	while(c != EOF ) {
		c = fgetc((FILE*)fp);
		//printf("%c\n", c);
		buff[l_num][i] = c;
		if(c == '\n') {
			l_num+=1;
			i = 0;
		} else {
			i++;
		}
	}
	buff[l_num][i] = 0;
	return i; 
}
int readlinen(FILE* fp) {
	char c = 0;
	int l_num = 0;
	// repeat until c is EOF
	while(c != EOF ) {
		c = fgetc((FILE*)fp);
		//printf("%c\n", c);
		if(c == '\n') {
			l_num+=1;
		} 
	}
	rewind(fp);
	return l_num+1; 
}
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
		int line_num = 0;

		/* input line */
		char* line = malloc(MAX_LINE_LENGTH);
		start_parser(exit_loop);

		/* introduction */
		printf("Scrawl Can Rarely Automate With Little-effort (scrawl)\nversion 0.5.4 - type help for general commands\n");
		
		/* main loop */
		while(strcmp(line, "exit\n") && !(*exit_loop)){
			/* setup line */
			clear_str(line, MAX_LINE_LENGTH);

			printf("%s", prompt);
			/* get line to parse */
			fgets(line, MAX_LINE_LENGTH, stdin);
			
			/* return value */
			variable rv;
			rv.t = TYPE_NUL;
			
			parse(line, &rv, line_num, 1);

			
			line_num++;	
			/* output based on type */
			if(rv.t == TYPE_INT8 || rv.t == TYPE_INT16 || rv.t == TYPE_INT32 || rv.t == TYPE_INT64){
				printf("%d\n", rv.value);
			} else if(rv.t == TYPE_BOOL){
				printf("%s\n", rv.value>0?"true":"false");	
			}
		}
		free(line);
		stop_parser();
	} else {
		char* filename = malloc(100);
		strcpy(filename, argv[1]);

		FILE* fp = fopen(filename, "r");
		int linenum = readlinen(fp);
		printf("%d\n", linenum);
		if(linenum == 0){
			printf("Error: file either does not exist or is empty");
			return 1;
		}
		char** buff = malloc(linenum);
		for(int i = 0; i < linenum; i++){
			buff[i] = malloc(200);
		}
		readfile(fp, buff);
		start_parser(exit_loop);
		for(int i = 0; i < linenum && strcmp(buff[i], "exit") && !(*exit_loop); i++){
			variable rv;
			rv.t = TYPE_NUL;
			parse(buff[i], &rv, i, 1);
		}
		stop_parser();
		fclose(fp);
	}
}