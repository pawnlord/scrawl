#include <stdio.h>
#include <stdlib.h>
#include <string.h>
#include "parse.h"
#define STR_SIZE 255
#define STRPTR_SIZE 255
line_structure ls;
int allocate_strptr(char*** strptr, int dim1, int dim2){
	(*strptr) = malloc(dim1);
	if((*strptr) == NULL){
		perror("allocate_strptr");
		return 0;
	}
	for(int i = 0; i < dim1; i++) {
		(*strptr)[i] = malloc(dim2);
		if((*strptr)[i] == NULL){
			perror("allocate_strptr");
			return 0;
		}
		for(int j = 0; j < dim2; j++) {
			(*strptr)[i][j] = 0;
		}
	}
}
void clear_strptr(char*** strptr) {
	for(int i = 0; strcmp((*strptr)[i], ""); i++){
		for(int j = 0; (*strptr)[i][j] != 0; j++){
			(*strptr)[i][j] = 0;
		}
	}
}

void init_ls(line_structure* ls) {
	allocate_strptr(&(ls->keywords), STRPTR_SIZE, STR_SIZE);
	allocate_strptr(&(ls->consts), STRPTR_SIZE, STR_SIZE);
	allocate_strptr(&(ls->vars), STRPTR_SIZE, STR_SIZE);
	ls->inited = 1;
}

int start_parser() {
	init_ls(&ls);
}

void reset_ls(line_structure* ls) {
	clear_strptr(&(ls->keywords));
	clear_strptr(&(ls->consts));
	clear_strptr(&(ls->vars));
}

void* parse(char* line) {
	static int   indentation_unit = 0;
	static int   last_indentation = 0;
	static char* alphanumeric = "qwertyuiopasdfghjklzxcvbnm1234567890_";
	static char* whitespace = " \t\n";
	int indentation = 0;
	
	char* current_token = malloc(100);
		
	for(int i = 0; i < 100; i++) {
		current_token[i] = 0;
	}
	reset_ls(&ls);
	char ct_counter = 0;
	char line_started = 0;
	for(int i = 0; line[i] != 0; i++) {
		//printf("%d, %s, %s, %c: %d, %d, %d\n", i, alphanumeric, whitespace, line[i], strchr(alphanumeric, line[i]), strchr(whitespace, line[i]), line_started  );
		if(strchr(alphanumeric, line[i]) != NULL) {
			current_token[ct_counter] = line[i];
			
			if(!line_started){
				if(indentation_unit == 0){
					indentation_unit = indentation;
				}
			}
			ct_counter++;
			line_started = 1;
		} else if(strchr(whitespace, line[i]) != NULL && !line_started) {
			indentation += 1;
		} else if(strchr(whitespace, line[i]) != NULL && line_started && strcmp(current_token, "")) {
			strcpy(ls.keywords[ls.keyword_num], current_token);
			printf("%s=%s\n", ls.keywords[ls.keyword_num], current_token);
			for(int j = 0; j < 100; j++) {
				current_token[j] = 0;
			}
			ct_counter = 0;
			ls.keyword_num++;
		}
	}
	return ls.keywords[1];
}