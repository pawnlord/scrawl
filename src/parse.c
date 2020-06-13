#include <stdio.h>
#include <stdlib.h>
#include <string.h>
#include <math.h>
#include "../inc/parse.h"
#define STR_SIZE 255
#define STRPTR_SIZE 255

state  master_state;
state  temp_state;
state* state_stack;
int current_state;

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

void init_variable(variable* var, int name_size){
	var->identifier = malloc(name_size);
	
	for(int i = 0; i < name_size; i++){
		var->identifier[i] = 0;
	}
	var->t = NUL;
}

int isnum(char c){
	static const char* numeric = "1234567890";
	return strchr(numeric, c) != NULL;
}

int create_onthefly_variable(variable* v){
	if(isnum(v->identifier[0])){
		v->value = (void*)atoi(v->identifier);
		if((int)v->value <= 255) v->t = INT8;
		else if((int)v->value <= pow(2, (sizeof(int)*8)+1)){ 
			if(sizeof(int) == 8) {
				v->t = INT64;
			} else{
				v->t = INT32;
			}
		} else{
			printf("SizeError: Number To Big! %d %d\n", (int)pow(2, (sizeof(int)*8)+1) , sizeof(int));
			exit(EXIT_FAILURE);
		}
		return 1;
	}
	return 0;
}

void initialize_states(int max_varnum, int max_connum){
	master_state.vars = malloc(max_varnum*sizeof(variable));
	for(int i = 0; i < max_varnum; i++){
		init_variable(&master_state.vars[i], STR_SIZE);
	}
	master_state.cons = malloc(max_connum*sizeof(variable));
	for(int i = 0; i < max_varnum; i++){
		init_variable(&master_state.cons[i], STR_SIZE);
	}
	master_state.var_num = 0;
	master_state.con_num = 0;
}

void init_ls(line_structure* ls) {
	allocate_strptr(&(ls->keywords), STRPTR_SIZE, STR_SIZE);
	ls->keyword_num = 0;
	ls->inited = 1;
}

int start_parser() {
	initialize_states(STRPTR_SIZE, STRPTR_SIZE);
}

void reset_ls(line_structure* ls) {
	clear_strptr(&(ls->keywords));
}

int str_in_varlist(char* str, variable* list){
	for(int i = 0; strcmp(list[i].identifier, ""); i++){
		if(strcmp(list[i].identifier, str) == 0){
			return i;
		}
	}
	return -1;
}

int autoset(variable* rval){
	int pointer;
	if((pointer = str_in_varlist(rval->identifier, master_state.vars)) != -1) {
		/* ^ check through variables */
		master_state.vars[pointer].value = rval->value;
		master_state.vars[pointer].t = rval->t;
	
	} else if ((str_in_varlist(rval->identifier, master_state.cons)) != -1 ||
		create_onthefly_variable(rval)){
			printf("TypeError: invalid lval of '=' operator (cannot be constant)\n", rval->identifier, rval->t);
	}
}

int getvar(variable* var){
	int pointer;

	if((pointer = str_in_varlist(var->identifier, master_state.vars)) != -1) {
		/* ^ check through variables */
		var->value =  master_state.vars[pointer].value;
		var->t =  master_state.vars[pointer].t;
	} else if((pointer = str_in_varlist(var->identifier, master_state.cons)) != -1) {
		/* ^ check through constants */
		var->value =  master_state.cons[pointer].value;
		var->t =  master_state.cons[pointer].t;
	} else {
		create_onthefly_variable(var);
	}
}

void parse(char* line, variable* return_value, int stop_at_symbol) {
	/* things we need to remember */
	static int   indentation_unit = 0;
	static int   last_indentation = 0;
	
	/* constant strings */
	static const char* alphanumeric = "qwertyuiopasdfghjklzxcvbnm1234567890_";
	
	static const char* symbols = "!@#$%^&*()-=+{}[]\\;:'\".,";
	
	/* TODO: Add priority */
	/*static const char* symbol_priority = "+-*";*/
	
	static const char* whitespace = " \t\n";
	
	line_structure ls;
	init_ls(&ls);
	
	/* default return flag */
	int default_val = 1;
	
	/* current indentation */
	int indentation = 0;
	
	/* token we are reading */
	char* current_token = malloc(100);
	/* clear it */	
	for(int i = 0; i < 100; i++) {
		current_token[i] = 0;
	}
					
	
	char ct_counter = 0;
	char line_started = 0;
	
	init_variable(return_value, 100);

	for(int i = 0; line[i] != 0; i++) {
		/* make sure line[i] is alphanumeric */
		if(strchr(alphanumeric, line[i]) != NULL) {
			/* copy */
			current_token[ct_counter] = line[i];
			
			/* if the line hasn't started, set indentation unit */
			if(!line_started){
				if(indentation_unit == 0){
					indentation_unit = indentation;
				}
			}

			ct_counter++;
			line_started = 1;
		
		} else if(strchr(whitespace, line[i]) != NULL && !line_started) {
			/* add to indentation*/
			
			indentation += 1;
		} else if(strchr(whitespace, line[i]) != NULL && line_started && strcmp(current_token, "")) {
			/* add keyword */
			strcpy(ls.keywords[ls.keyword_num], current_token);
			
			ls.keyword_num++;
			
			for(int j = 0; j < 100; j++) {
				current_token[j] = 0;
			}
		
			ct_counter = 0;
		
		} else if(strchr(symbols, line[i]) != NULL && !stop_at_symbol){
			/* symbol found, use it */
			/* if line hasn't started, do things specific to that*/
			if(!line_started){
				if(line[i] == '-'){
					variable rtemp;
					
					/* recursive, check rvalue */
					parse(line+i+1, &rtemp, 1);
					if(rtemp.t != INT8 && rtemp.t != INT16 && rtemp.t != INT32){
						printf("TypeError: invalid rval of '-' operator %s (%d) of type %d (needs to be int)\n", rtemp.identifier, rtemp.value, rtemp.t);
						
						return_value->value = 0;
						return_value->t  = 0;
						
						default_val = 0;
						break;	
					}
					sprintf(return_value->identifier, "%d", -((int)rtemp.value));
					
					return_value->value = (void*)(-((int)rtemp.value));
					return_value->t = INT32;
					
					default_val = 0;
				}
			} else {
				/* symbol found */
				if(strcmp(current_token, "")){
					/* symbols also act as whitespace */
					strcpy(ls.keywords[ls.keyword_num], current_token);
					ls.keyword_num++;
				}
				for(int j = 0; j < 100; j++) {
					current_token[j] = 0;
				}
				ct_counter = 0;
				
				/* += operator style */
				char is_autoset = 0;

				if(line[i+1] == '='){
					is_autoset = 1;
				}

				/* actual functionality */
				if(line[i] == '+'){
					variable rtemp;
					parse(line+i+1+is_autoset, &rtemp, 1);

					if(rtemp.t != INT8 && rtemp.t != INT16 && rtemp.t != INT32){
						printf("TypeError: invalid rval of '+' operator %s of type %d (needs to be int)\n", rtemp.identifier, rtemp.t);
						
						return_value->value = 0;
						return_value->t  = 0;

						default_val = 0;
						break;	
					}

					if(strcmp(ls.keywords[0], "")) {
						strcpy(return_value->identifier, ls.keywords[0]);
						getvar(return_value);
					}

					sprintf(return_value->identifier, "%d", (int)return_value->value + (int)rtemp.value);
					return_value->value = (void*)((int)rtemp.value + (int)return_value->value);

					default_val = 0;

					if(is_autoset == 1 && strcmp(ls.keywords[0], "")){
						strcpy(return_value->identifier, ls.keywords[0]);
						autoset(return_value);
					}

				}
				if(line[i] == '*'){
					variable rtemp;
					
					parse(line+i+1+is_autoset, &rtemp, 1);
					if(rtemp.t != INT8 && rtemp.t != INT16 && rtemp.t != INT32){
						printf("TypeError: invalid rval of '*' operator %s of type %d (needs to be int)\n", rtemp.identifier, rtemp.t);
						
						return_value->value = 0;
						return_value->t  = 0;
						
						default_val = 0;
						
						break;	
					}

					if(strcmp(ls.keywords[0], "") && default_val) {
						strcpy(return_value->identifier, ls.keywords[0]);
						getvar(return_value);
					}

					sprintf(return_value->identifier, "%d", (int)return_value->value * (int)rtemp.value);
					return_value->value = (void*)((int)rtemp.value * (int)return_value->value);

					default_val = 0;

					if(is_autoset == 1 && strcmp(ls.keywords[0], "")){
						strcpy(return_value->identifier, ls.keywords[0]);
						autoset(return_value);
					}
				}
				if(line[i] == '-'){
					variable rtemp;
					parse(line+i+1+is_autoset, &rtemp, 1);
					if(rtemp.t != INT8 && rtemp.t != INT16 && rtemp.t != INT32){
						printf("TypeError: invalid rval of '-' operator %s of type %d (needs to be int)\n", rtemp.identifier, rtemp.t);
						
						return_value->value = 0;
						return_value->t  = 0;
						
						default_val = 0;
						break;	
					}

					if(strcmp(ls.keywords[0], "") && default_val) {
						strcpy(return_value->identifier, ls.keywords[0]);
						getvar(return_value);
					}
					sprintf(return_value->identifier, "%d", (int)return_value->value - (int)rtemp.value);
					return_value->value = (void*)((int)return_value->value - (int)rtemp.value);
					
					default_val = 0;
					
					if(is_autoset == 1 && strcmp(ls.keywords[0], "")){
						strcpy(return_value->identifier, ls.keywords[0]);
						autoset(return_value);
					}
				}
				if(line[i] == '='){
					variable rtemp;
					
					parse(line+i+1, &rtemp, 0);
					if(rtemp.t != INT8 && rtemp.t != INT16 && rtemp.t != INT32){
						/* rvalue bad, error, set all to zero */
						printf("TypeError: invalid rval of '=' operator %s of type %d\n", rtemp.identifier, rtemp.t);
						
						return_value->value = 0;
						return_value->t  = 0;
						
						default_val = 0;
						break;	
					}
					if(ls.keyword_num > 1){
						/* too many lvals, set all to zero */
						printf("TypeError: invalid lval of '=' operator (cannot have multiple)\n", rtemp.identifier, rtemp.t);
						
						return_value->value = 0;
						return_value->t  = 0;
						
						default_val = 0;
						break;	
					}
					if(strcmp(ls.keywords[0], "") && default_val) {
						strcpy(return_value->identifier, ls.keywords[0]);
						int pointer;
						
						if((pointer = str_in_varlist(return_value->identifier, master_state.vars)) != -1) {
							/* ^ check through variables */
							
							/* we found one, copy new value */
							master_state.vars[pointer].value = rtemp.value;
							master_state.vars[pointer].t = rtemp.t;
							
							return_value->value = rtemp.value;
							return_value->t  = rtemp.t;
						} else if((pointer = str_in_varlist(return_value->identifier, master_state.cons)) != -1 ||
								create_onthefly_variable(return_value)) {
							/* error, constant used */
							printf("TypeError: invalid lval of '=' operator (cannot be constant)\n", rtemp.identifier, rtemp.t);
							
							/* set all to zero */
							return_value->value = 0;
							return_value->t  = 0;
							
							default_val = 0;

							break;	
						} else {
							/* push back rval */
							strcpy(master_state.vars[master_state.var_num].identifier, return_value->identifier);
							
							master_state.vars[master_state.var_num].value = rtemp.value;
							master_state.vars[master_state.var_num].t = rtemp.t;
							
							return_value->value = rtemp.value;
							return_value->t  = rtemp.t;
							
							master_state.var_num+=1;
						}
					}
					default_val = 0;
					break;
				}
				/* comment */
				if(line[i] == '#') {
					/* add keyword */
					strcpy(ls.keywords[ls.keyword_num], current_token);
					ls.keyword_num++;
					
					for(int j = 0; j < 100; j++) {
						current_token[j] = 0;
					}
				
					ct_counter = 0;
					/* comment found, line end */
					break;
				}
			}
		} else if(strchr(symbols, line[i]) != NULL && stop_at_symbol) {
			/* we are supposed to stop at a symbol! */
			if(strcmp(current_token, "")){
				/* symbols also act as whitespace */
				strcpy(ls.keywords[ls.keyword_num], current_token);
				ls.keyword_num++;
			}
			break;
		}
	}
	/* find value of keyword */
	if(strcmp(ls.keywords[0], "") && default_val) {
		strcpy(return_value->identifier, ls.keywords[0]);
		getvar(return_value);
	} 
}