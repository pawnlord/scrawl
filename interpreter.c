#include <stdio.h>
#include <stdlib.h>
#include "parse.h"
#define MAX_LINE_LENGTH 100
char* prompt = "scripts>";

void clear_str(char* str, int size){
	for(int i = 0; i < size; i++){
		str[i] = 0;
	}
}


int main(int argc, char** argv){
	int mode = argc > 1;
	if(mode == 0){
		char* line = malloc(MAX_LINE_LENGTH);
		clear_str(line, MAX_LINE_LENGTH);
		start_parser();
		printf("This is simple crappy resource intensive potential trainwreck script (scripts)\nversion 1.0.0 - type help for general commands\n");
		
		printf("%s", prompt);
		fgets(line, MAX_LINE_LENGTH, stdin);
		variable rv;
		parse(line, &rv);
		printf("%s\n", rv.identifier);
		
		free(line);
	}
}