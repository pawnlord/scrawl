#include <stdio.h>
#include <stdlib.h>
#include <string.h>
#include <math.h>
#include "../inc/parse.h"
#include "../inc/functions.h"
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

void free_strptr(char*** strptr, int dim1) {
	for(int i = 0; i < dim1; i++){
		free((*strptr)[i]);
	}
	free((*strptr));
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
	var->t = TYPE_NUL;
}

int isnum(char c){
	static const char* numeric = "1234567890";
	return strchr(numeric, c) != NULL;
}

int isbool(char* s){
	return (strcmp(s, "true") == 0 || strcmp(s, "false") == 0);
}

int isstring(char* s){
	return (s[0] == '"');
}

int create_onthefly_variable(variable* v){
	if(strcmp("", v->identifier) == 0){
		v->value = 0;
		v->t = TYPE_NUL;
	}
	if(isnum(v->identifier[0])){
		v->value = (void*)atoi(v->identifier);
		if((int)v->value <= 255) v->t = TYPE_INT8;
		else if((int)v->value <= pow(2, (sizeof(int)*8)+1)){ 
			if(sizeof(int) == 8) {
				v->t = TYPE_INT64;
			} else{
				v->t = TYPE_INT32;
			}
		} else{
			printf("SizeError: Number To Big! %d %d\n", (int)pow(2, (sizeof(int)*8)+1) , sizeof(int));
			exit(EXIT_FAILURE);
		}
		return 1;
	}
	if(isbool(v->identifier)){
		v->value = (void*)(strcmp(v->identifier, "false") == 0 ? 0 : 1);
		v->t = TYPE_BOOL;	
	}
	if(isstring(v->identifier)){
		int length = strlen(v->identifier);
		char* s = malloc(length-2);
		if(s!=NULL){
			char* temp = malloc(length+1);
			strcpy(temp, v->identifier+1);
			temp[length-2] = '\0';
			strcpy(s, temp);
			v->value = s;
			v->t = TYPE_STRING;
			free(temp);
			return 1;
		}
	}
	return 0;
}

void initialize_states(int max_varnum, int max_connum, int max_block, volatile int* exit_loop){
	master_state.vars = malloc(max_varnum*sizeof(variable));
	for(int i = 0; i < max_varnum; i++){
		init_variable(&master_state.vars[i], STR_SIZE);
	}
	master_state.cons = malloc(max_connum*sizeof(variable));
	for(int i = 0; i < max_connum; i++){
		init_variable(&master_state.cons[i], STR_SIZE);
	}
	master_state.var_num = 0;
	
	master_state.con_num = 0;
	
	master_state.block_line_num = malloc(max_block*sizeof(int));
	for(int i = 0; i < max_block; i++){
		master_state.block_line_num[i] = 0;
	}
	master_state.lines = malloc(5000);
	
	master_state.block_level = 0;

	master_state.can_unindent = 0;

	master_state.line_count = 0;
	
	master_state.block_types = malloc(max_block*sizeof(int));
	for(int i = 0; i < max_block; i++){
		master_state.block_types[i] = 0;
	}
	master_state.last_indent = 0;

	master_state.indent_unit = 0;

	master_state.running_block = 1;

	master_state.stop_comparison= 0;
	master_state.exit_loop= exit_loop;
	master_state.functions = (function_t*)(malloc(255*sizeof(function_t)));
	make_function(&master_state.functions[0], funcprint, NULL, "print"); /* TODO: REMEMBER THAT NULL MEANS IT TAKES ALL POSSIBLE ARGUEMENTS*/
	
	make_function(&master_state.functions[1], funcsystem, NULL, "system"); /* TODO: REMEMBER THAT NULL MEANS IT TAKES ALL POSSIBLE ARGUEMENTS*/
	
	make_function(&master_state.functions[2], (function)NULL, NULL, ""); /* TODO: REMEMBER THAT NULL MEANS IT TAKES ALL POSSIBLE ARGUEMENTS*/
		
}

void init_ls(line_structure* ls) {
	allocate_strptr(&(ls->keywords), STRPTR_SIZE, STR_SIZE);
	ls->keyword_num = 0;
	ls->inited = 1;
}

int start_parser(volatile int* exit_loop) {
	initialize_states(STRPTR_SIZE, STRPTR_SIZE, STR_SIZE, exit_loop);
}

int stop_parser() {
	free(master_state.vars);
	free(master_state.cons);
	free(master_state.block_line_num);
	for(int i = 0; i < master_state.line_count; i++){
		free(master_state.lines[i]);
	}
	if(master_state.lines != NULL){
		free(master_state.lines);
	} else{
		printf("line was null\n");
	}
	free(master_state.block_line_num);
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
int str_in_funclist(char* str, function_t* list){
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

/* Split line into main keywords and assign types */
int tokenize(char* line, token** tokens){
	int current_token = 0;
	int current_character = 0;
	(*tokens) = (token*)malloc(sizeof(token) * 100);
	(*tokens)[current_token].identifier = malloc(STR_SIZE);
	
	/* constant strings */
	static const char* alphanumeric = "qwertyuiopasdfghjklzxcvbnm1234567890_";	
	static const char* numeric = "1234567890";	
	static const char* symbols = "!@#$%^&*()-=+{}[]\\;:'\"<>.,";
	static const char* whitespace = " \t\n";
	
	/* current indentation */
	int indentation = 0;
	int line_started = 0;
	enum {
		LAST_NONE = 0,
		LAST_ALPHA,
		LAST_NUMBER,
		LAST_OP,
		LAST_WHITESPACE
	} last_char_type = LAST_NONE;
	/* loop through every character */
	for(int i = 0; line[i] != 0; i++){
		/* see if it's alphanumeric, a symbol, or whitespace */
		if(strchr(alphanumeric, line[i]) != NULL) {
			/* if it was a different type of character last, push back and start a new token 
			   else if it's just started or one was already made by whitespace, just set the type */
			if(last_char_type == LAST_OP || (last_char_type == LAST_WHITESPACE && !line_started)) {
				
				/* null terminate */
				(*tokens)[current_token].identifier[current_character] = 0;

				/* reset and pushback */
				current_character = 0;
				current_token++;
				
				/* allocate and set type */
				(*tokens)[current_token].identifier = malloc(current_token);
				(*tokens)[current_token].ttype = TOKEN_VAR;
			} else if(last_char_type == LAST_NONE || last_char_type == LAST_WHITESPACE){
				(*tokens)[current_token].ttype = TOKEN_VAR;
			}
			
			/* last char depending on if it is number or not */
			if(strchr(numeric, line[i]) != NULL){
				last_char_type = LAST_NUMBER;
			} else {
				last_char_type = LAST_ALPHA;
			}

			/* copy character */
			(*tokens)[current_token].identifier[current_character] = line[i];
			current_character++;

			/* start line if it's not */
			line_started = 1;
		} else if(strchr(whitespace, line[i]) != NULL) {
			/* if inbetween characters, set up a new token */
			if(last_char_type == LAST_ALPHA || last_char_type == LAST_NUMBER || last_char_type == LAST_OP) {
				(*tokens)[current_token].identifier[current_character] = 0;
				
				current_character = 0;
				current_token++;
				
				(*tokens)[current_token].identifier = malloc(current_token);
			}
			/* if the line hasn't started, add to indentation */
			if(!line_started){
				if(last_char_type == LAST_NONE) {
					(*tokens)[current_token].ttype = TOKEN_WHITESPACE;
				}
				indentation+=1;

				(*tokens)[current_token].identifier[current_character] = line[i];
				current_character++;
			}

			/* set last_char_type accordingly */
			last_char_type = LAST_WHITESPACE;			
		} else if(strchr(symbols, line[i]) != NULL) {
			/* if after letter or number, make new token */
			if(last_char_type == LAST_ALPHA || last_char_type == LAST_NUMBER) {
				(*tokens)[current_token].identifier[current_character] = 0;
				
				current_character = 0;
				current_token++;
				
				(*tokens)[current_token].identifier = malloc(current_token);
				(*tokens)[current_token].ttype = TOKEN_OP;
			}
			
			/* if one is there for us, just set the type */
			if(last_char_type == LAST_NONE || last_char_type == LAST_WHITESPACE){
				(*tokens)[current_token].ttype = TOKEN_OP;
			}
			
			
			/* copy */
			(*tokens)[current_token].identifier[current_character] = line[i];
			current_character++;
			
			

			/* set last_char_type accordingly */
			last_char_type = LAST_OP;

			if(line[i] == '=' && line[i+1] != '='){
				(*tokens)[current_token].identifier[current_character] = 0;
				
				current_character = 0;
				current_token++;
				
				(*tokens)[current_token].identifier = malloc(current_token);
				(*tokens)[current_token].ttype = TOKEN_VAR;
				
			}
			/* if it's a string, it's different*/
			if(line[i] == '"') {
				/* separate it from the last operator */
				if(last_char_type == LAST_OP){
					current_character--;
					(*tokens)[current_token].identifier[current_character] = 0;
					
					current_character = 0;
					current_token++;
					
					(*tokens)[current_token].identifier = malloc(current_token);
					/* recopy the character */
					(*tokens)[current_token].identifier[current_character] = line[i];
					current_character++;
				}

				last_char_type=LAST_ALPHA;
				(*tokens)[current_token].ttype = TOKEN_VAR;
				
				/* copy characters */
				for(i++; ; i++){
					if(line[i] == 0){
						printf("TokenError: Unended string %s\n", (*tokens)[current_token].identifier);
						return 0;
					}
				
					(*tokens)[current_token].identifier[current_character] = line[i];
					current_character++;
				
					if(line[i] == '"'){
						break;
					}
				}
			}

			/* start line if not started */
			line_started = 1;
		}
	}
	/* add a token that terminates */
	(*tokens)[current_token].ttype = TOKEN_END;
			
	return 1;
}

/* Used to see if - sign is negative or subtraction */
int last_token_is_number(token* tokens, int index){
	if(index == 0){
		return 0;
	}
	return 1;
}

/***********************
 * ACTUAL PARSER START *
 ***********************/

/* put all samey code together for ease of reading and access */
int add( token* tokens, variable* return_value, int line_num, int i){
	variable rtemp;
	if(i == 0){
		/* TODO: don't do this */
		printf("TokenError: No lval for '+' (line num %d)\n", line_num);
		
		return_value->value = 0;
		return_value->t  = 0;

		return 0;
	}	
	int temp_stop_comparison = master_state.stop_comparison;

	master_state.stop_comparison = 1;
	parse_tokens(tokens+i+1, &rtemp, line_num);
	master_state.stop_comparison =  temp_stop_comparison;
	

	if(rtemp.t != return_value->t){
		printf("TypeError: rval and lval types don't match for '+'! (line num %d)\n",line_num);
		return_value->value = 0;
		return_value->t  = 0;

		return 0;	
	}

	if(strcmp(tokens[i-1].identifier, "")) {
		strcpy(return_value->identifier, tokens[i-1].identifier);
		getvar(return_value);
	}
	
	if(return_value->t == TYPE_STRING){
		char* temp = malloc(strlen(return_value->identifier) + strlen(rtemp.identifier) + 1);

		strcpy(temp, return_value->value);
		strcat(temp, rtemp.value);

		sprintf(return_value->identifier, "\"%s\"", temp);
		return_value->value = temp;

	} else if(return_value->t == TYPE_INT8 || return_value->t == TYPE_INT16 || 
			return_value->t == TYPE_INT32 || return_value->t == TYPE_INT64 ){
		sprintf(return_value->identifier, "%d", (int)return_value->value + (int)rtemp.value);
		return_value->value = (void*)((int)rtemp.value + (int)return_value->value);
	} else {
		printf("TypeError: type %d unimplement for '+'! (line num %d)\n", return_value->t, line_num);
		
		return_value->value = 0;
		return_value->t  = 0;

		return 0;	
	}
	return 1;
}

int subtract( token* tokens, variable* return_value, int line_num, int i){
	variable rtemp;
	if(i == 0){
		/* TODO: don't do this */
		printf("TokenError: - needs lval (line num %d)\n", line_num);
		
		return_value->value = 0;
		return_value->t  = 0;

		return 0;	
	}	
	int temp_stop_comparison = master_state.stop_comparison;
	master_state.stop_comparison = 1;
	parse_tokens(tokens+i+1, &rtemp, line_num);
	master_state.stop_comparison =  temp_stop_comparison;

	if(rtemp.t != TYPE_INT8 && rtemp.t != TYPE_INT16 && rtemp.t != TYPE_INT32){
		printf("TypeError: invalid rval of '-' operator %s of type %d (needs to be int) (line num %d)\n", rtemp.identifier, rtemp.t, line_num);
		
		return_value->value = 0;
		return_value->t  = 0;

		return 0;	
	}

	if(strcmp(tokens[i-1].identifier, "")) {
		strcpy(return_value->identifier, tokens[i-1].identifier);
		getvar(return_value);
	}
	
	// printf("%d - %d = %d\n", (int)return_value->value , (int)rtemp.value, (int)return_value->value - (int)rtemp.value);
	sprintf(return_value->identifier, "%d", (int)return_value->value - (int)rtemp.value);
	return_value->value = (void*)((int)return_value->value - (int)rtemp.value);
	return 1;
}

int multiply( token* tokens, variable* return_value, int line_num, int i){
	variable rtemp;
	if(i == 0){
		/* TODO: don't do this */
		printf("TokenError: No lval for '*' operator (line num %d)\n", line_num);
		
		return_value->value = 0;
		return_value->t  = 0;

		return 0;	
	}	
	int temp_stop_comparison = master_state.stop_comparison;
	master_state.stop_comparison = 1;
	parse_tokens(tokens+i+1, &rtemp, line_num);
	master_state.stop_comparison = temp_stop_comparison;
	

	if(rtemp.t != TYPE_INT8 && rtemp.t != TYPE_INT16 && rtemp.t != TYPE_INT32){
		printf("TypeError: invalid rval of '*' operator %s of type %d (needs to be int) (line num %d)\n", rtemp.identifier, rtemp.t, line_num);
		
		return_value->value = 0;
		return_value->t  = 0;

		return 0;	
	}

	if(strcmp(tokens[i-1].identifier, "")) {
		strcpy(return_value->identifier, tokens[i-1].identifier);
		getvar(return_value);
	}

	sprintf(return_value->identifier, "%d", (int)return_value->value * (int)rtemp.value);
	return_value->value = (void*)((int)rtemp.value * (int)return_value->value);
	
	return 1;
}

int less_than( token* tokens, variable* return_value, int line_num, int i){
	variable rtemp;
	if(i == 0){
		/* TODO: don't do this */
		printf("TokenError: No lval for '<' operator (line num %d)\n", line_num);
		
		return_value->value = 0;
		return_value->t  = 0;

		return 0;	
	}	
	parse_tokens(tokens+i+1, &rtemp, line_num);
	

	if(rtemp.t != TYPE_INT8 && rtemp.t != TYPE_INT16 && rtemp.t != TYPE_INT32){
		printf("TypeError: invalid rval of '<' operator %s of type %d (needs to be int) (line num %d)\n", rtemp.identifier, rtemp.t, line_num);
		
		return_value->value = 0;
		return_value->t  = 0;

		return 0;	
	}

	if(strcmp(tokens[i-1].identifier, "")) {
		strcpy(return_value->identifier, tokens[i-1].identifier);
		getvar(return_value);
	}
	
	sprintf(return_value->identifier, "%d", (int)return_value->value < (int)rtemp.value);
	return_value->value = (void*)((int)return_value->value < (int)rtemp.value);
	return_value->t = TYPE_BOOL;

	return 1;
}
int greater_than( token* tokens, variable* return_value, int line_num, int i){
	variable rtemp;
	if(i == 0){
		/* TODO: don't do this */
		printf("TokenError: No lval for '>' operator (line num %d)\n", line_num);
		
		return_value->value = 0;
		return_value->t  = 0;

		return 0;	
	}	
	parse_tokens(tokens+i+1, &rtemp, line_num);
	

	if(rtemp.t != TYPE_INT8 && rtemp.t != TYPE_INT16 && rtemp.t != TYPE_INT32){
		printf("TypeError: invalid rval of '>' operator %s of type %d (needs to be int) (line num %d)\n", rtemp.identifier, rtemp.t, line_num);
		
		return_value->value = 0;
		return_value->t  = 0;

		return 0;	
	}

	if(strcmp(tokens[i-1].identifier, "")) {
		strcpy(return_value->identifier, tokens[i-1].identifier);
		getvar(return_value);
	}

	sprintf(return_value->identifier, "%d", (int)return_value->value > (int)rtemp.value);
	return_value->value = (void*)((int)return_value->value > (int)rtemp.value);
	return_value->t = TYPE_BOOL;

	return 1;
}
int equal_to( token* tokens, variable* return_value, int line_num, int i){
	variable rtemp;
	if(i == 0){
		/* TODO: don't do this */
		printf("TokenError: No lval for '==' operator (line num %d)\n", line_num);
		
		return_value->value = 0;
		return_value->t  = 0;

		return 0;	
	}	
	parse_tokens(tokens+i+1, &rtemp, line_num);
	

	if(rtemp.t != TYPE_INT8 && rtemp.t != TYPE_INT16 && rtemp.t != TYPE_INT32){
		printf("TypeError: invalid rval of '==' operator %s of type %d (needs to be int) (line num %d)\n", rtemp.identifier, rtemp.t, line_num);
		
		return_value->value = 0;
		return_value->t  = 0;

		return 0;	
	}

	if(strcmp(tokens[i-1].identifier, "")) {
		strcpy(return_value->identifier, tokens[i-1].identifier);
		getvar(return_value);
	}

	sprintf(return_value->identifier, "%d", (int)return_value->value == (int)rtemp.value);
	return_value->value = (void*)((int)return_value->value == (int)rtemp.value);
	return_value->t = TYPE_BOOL;

	return 1;
}


int run_if(token* tokens, variable* return_value, int line_num){
	char* if_line = malloc(STR_SIZE);
	int if_line_number = master_state.block_line_num[master_state.block_level];
	strcpy(if_line, master_state.lines[if_line_number]);
	
	token* if_tokens;
	tokenize(if_line, &if_tokens);
	
	int i;
	for(i = 0; if_tokens[i].ttype != TOKEN_END; i++);
	
	if(i == 0) {
		/* huh? */
		printf("IndentError: indent not in block (line num %d)\n", line_num);
		return_value->value = 0;
		return_value->t  = 0;
		return 0;	
	}
	
	if_tokens[i-1].ttype = TOKEN_END;
	
	int temp_level = master_state.block_level;
	master_state.block_level = 0;

	variable evaled_expression;
	parse_tokens(if_tokens+1, &evaled_expression, if_line_number);
	
	master_state.block_level = temp_level;
	
	if(evaled_expression.value){
		variable ret;
		for (int i = if_line_number+1; i < line_num; i++){
			parse(master_state.lines[i], &ret, i, 0);
		}
	} 

	free(if_line);
	return 1;
}

int run_while(token* tokens, variable* return_value, int line_num){
	char* while_line = malloc(STR_SIZE);
	int while_line_number = master_state.block_line_num[master_state.block_level];
	strcpy(while_line, master_state.lines[while_line_number]);
	
	token* while_tokens;
	tokenize(while_line, &while_tokens);
	
	int i;
	for(i = 0; while_tokens[i].ttype != TOKEN_END; i++);
	
	if(i == 0) {
		/* huh? */
		printf("IndentError: indent not in block (line num %d)\n", line_num);
		return_value->value = 0;
		return_value->t  = 0;
		return 0;	
	}
	
	while_tokens[i-1].ttype = TOKEN_END;
	
	int temp_level = master_state.block_level;
	master_state.block_level = 0;

	variable evaled_expression;
	variable ret;
	parse_tokens(while_tokens+1, &evaled_expression, while_line_number);
	while(evaled_expression.value && !(*master_state.exit_loop)){
		
		master_state.block_level = temp_level;
	
		for (int i = while_line_number+1; i < line_num; i++){
			parse(master_state.lines[i], &ret, i, 0);
		}
		parse_tokens(while_tokens+1, &evaled_expression, while_line_number);
		
	} 

	free(while_line);
	return 1;
}

int parse_tokens(token* tokens, variable* return_value, int line_num){
	init_variable(return_value, 100);
	int is_autoset;
	block_type block = BLOCK_NONE;
	int i;
	if(master_state.running_block){
		for(i = 0; tokens[i].ttype != TOKEN_END; i++){
			is_autoset = 0;
			if(strcmp(tokens[i].identifier, "_debugblocks") == 0){
				printf("current block info:\nblock_level: %d\nblock_line_num: %d\nblock_type: %d",
					master_state.block_level, master_state.block_line_num[master_state.block_level], 
					master_state.block_types[master_state.block_level]);
				return 0;
			}
			if(tokens[i].ttype == TOKEN_VAR) {
				if(strcmp(tokens[i].identifier, "if") == 0){
					block = BLOCK_IF;
					return_value->t=TYPE_NUL;
				} else if(strcmp(tokens[i].identifier, "while") == 0){
					block = BLOCK_WHILE;
					return_value->t=TYPE_NUL;
				} else if(str_in_funclist(tokens[i].identifier, master_state.functions) >= 0) {
					int len = 0;
					int fname_index = i;
					int is_beginning = 1;
					int beginning, end, param_num = 0;
					int k = 0;
					variable* v ;
					
					for(k = i+1; tokens[k].ttype != TOKEN_END; k++){
						if(strcmp(tokens[k].identifier, ",") == 0 || strcmp(tokens[k].identifier, ")") == 0){
							len += 1;
						}
					}
					v = (variable*)malloc(len*sizeof(variable));
					k = 0;
					for(k = i+1; tokens[k].ttype != TOKEN_END; k++){
						if(is_beginning){
							if(strcmp(tokens[k].identifier, "(") != 0){
								/* no opener, error */
								printf("SyntaxError: Expected '(' (line num %d).\n", line_num);

								return 0;
							}
							beginning = k;
							is_beginning = 0;
						}
						else if(tokens[k].ttype == TOKEN_WHITESPACE){
							if(strcmp(tokens[beginning].identifier, ",") == 0){
								beginning = k;
							}
						}
						else if(strcmp(tokens[k].identifier, ",") == 0 || strcmp(tokens[k].identifier, ")") == 0){
							end = k;
							token temp = tokens[end];
							tokens[end].ttype = TOKEN_END;
							parse_tokens(tokens+beginning, &(v[param_num]), line_num);
							param_num++;
							tokens[end].ttype = temp.ttype;
							beginning = k;
						}
					}
					
					v[param_num].t = TYPE_NUL;
					for(int j = 0; master_state.functions[j].f != NULL; j++){
						
						if(strcmp(master_state.functions[j].identifier, tokens[fname_index].identifier) == 0) {
							master_state.functions[j].f(&v);
						}
					}
					for(int j = 0; v[j].t = TYPE_NUL; j++ ){
						free(v[j].identifier);
					}
					free(v);
					return 0;
				} else {
					/* get value if it is a variable */
					strcpy(return_value->identifier, tokens[i].identifier);
					getvar(return_value);
				}
			} else if (tokens[i].ttype == TOKEN_OP){
				if (strcmp(tokens[i].identifier, ":") == 0){

					return_value->value = 0;
					return_value->t  = 0;
					if(block == BLOCK_NONE){
						/* no block, error */
						printf("SyntaxError: No block started (line num %d).\n", line_num);

						return 0;
					}

					master_state.block_level++;

					master_state.block_types[master_state.block_level] = block;
					master_state.block_line_num[master_state.block_level] = line_num;
					master_state.can_unindent = 0;
					if(block == BLOCK_IF || block == BLOCK_WHILE){
						master_state.running_block = 0;
					}

				} else if(strcmp(tokens[i].identifier, "=") == 0 && block == BLOCK_NONE) {
					variable rtemp;
					
					parse_tokens(tokens+i+1, &rtemp, line_num);
					
					if(i == 0){
						/* too many lvals, set all to zero */
						printf("TokenError: No lval for '=' operator (line num %d)\n", line_num);
						
						return_value->value = 0;
						return_value->t  = 0;
						
						break;	
					}

					if(strcmp(tokens[i-1].identifier, "")) {
						strcpy(return_value->identifier, tokens[i-1].identifier);
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
							printf("TypeError: invalid lval of '=' operator (cannot be constant) (line num %d)\n", rtemp.identifier, rtemp.t, line_num);
							
							/* set all to zero */
							return_value->value = 0;
							return_value->t  = 0;

							break;	
						} else {
							/* push back rval */
							/* make new variable */
							strcpy(master_state.vars[master_state.var_num].identifier, return_value->identifier);
							
							master_state.vars[master_state.var_num].value = rtemp.value;
							master_state.vars[master_state.var_num].t = rtemp.t;
							
							/* set return value */
							return_value->value = rtemp.value;
							return_value->t  = rtemp.t;
							
							master_state.var_num+=1;
						}

					}
					break;

				}
				else if((strcmp(tokens[i].identifier, "+") == 0 ||
						(is_autoset = !strcmp(tokens[i].identifier, "+="))) && block == BLOCK_NONE) {

					if (!add(tokens, return_value, line_num, i)){
						break;
					}

					if(is_autoset){
						strcpy(return_value->identifier, tokens[i-1].identifier);
						autoset(return_value);
					}
					break;
				}else if((strcmp(tokens[i].identifier, "*") == 0 ||
						(is_autoset = !strcmp(tokens[i].identifier, "*="))) && block == BLOCK_NONE) {

					
					if (!multiply(tokens, return_value, line_num, i)){
						break;
					}

					if(is_autoset){
						strcpy(return_value->identifier, tokens[i-1].identifier);
						autoset(return_value);
					}
					break;
				} else if (strcmp(tokens[i].identifier, "-") == 0 && !last_token_is_number(tokens, i) && block == BLOCK_NONE) {
					i++;
					variable rtemp;
					init_variable(&rtemp, 100);
					strcpy(rtemp.identifier, tokens[i].identifier);
					
					getvar(&rtemp);
					/* TypeError: wrong Type */
					if(rtemp.t != TYPE_INT8 && rtemp.t != TYPE_INT16 && rtemp.t != TYPE_INT32){
						printf("TypeError: invalid rval of '-' operator %s (%d) of type %d (needs to be int) (line num %d)\n", rtemp.identifier, rtemp.value, rtemp.t, line_num);
						
						return_value->value = 0;
						return_value->t  = 0;
						
						break;	
					}
					sprintf(return_value->identifier, "%d", -((int)rtemp.value));
					
					return_value->value = (void*)(-((int)rtemp.value));
					return_value->t = TYPE_INT32;
					
					strcpy(tokens[i].identifier, return_value->identifier);

				} else if((strcmp(tokens[i].identifier, "-") == 0 ||
						(is_autoset = !strcmp(tokens[i].identifier, "-="))) && block == BLOCK_NONE) {

					if (!subtract(tokens, return_value, line_num, i)){
						break;
					}

					if(is_autoset){
						strcpy(return_value->identifier, tokens[i-1].identifier);
						autoset(return_value);
					}
					break;
				} else if(master_state.stop_comparison){
					break;
				} else if(strcmp(tokens[i].identifier, "<") == 0 && block == BLOCK_NONE) {

					if (!less_than(tokens, return_value, line_num, i)){
						break;
					}

					break;
				} else if(strcmp(tokens[i].identifier, ">") == 0 && block == BLOCK_NONE) {

					if (!greater_than(tokens, return_value, line_num, i)){
						break;
					}
					break;
				} else if(strcmp(tokens[i].identifier, "==") == 0 && block == BLOCK_NONE) {

					if (!equal_to(tokens, return_value, line_num, i)){
						break;
					}

					break;
				} 

			} else if (tokens[i].ttype == TOKEN_WHITESPACE && i == 0){
				if(master_state.block_level == 0){
					/* if we're not in a block, what are we doing? */
					printf("IndentError: indent not in block (line num %d)\n", line_num);

					return_value->value = 0;
					return_value->t  = 0;

					break;	
				}
			}
		}
	} else {
		int block_ended = 0, temp_lastindent = 0;
		for(i = 0; tokens[i].ttype != TOKEN_END; i++){
			if(strcmp(tokens[i].identifier, "_debugblocks") == 0){
				printf("current block info:\nblock_level: %d\nblock_line_num: %d\nblock_type: %d",
					master_state.block_level, master_state.block_line_num[master_state.block_level], 
					master_state.block_types[master_state.block_level]);
				return 0;
			}
			if (tokens[i].ttype == TOKEN_WHITESPACE && i == 0){
				if(master_state.block_level == 0){
					/* if we're not in a block, what are we doing? */
					printf("IndentError: indent not in block (line num %d)\n", line_num);

					return_value->value = 0;
					return_value->t  = 0;

					break;	
				}
				/* get indentation level */
				int indentation = strlen(tokens[i].identifier);
				/* set indent unit if we are in a new block */
				if(master_state.last_indent == 0 ) {
					master_state.indent_unit = indentation;
				}
				/*check if indentation matches with unit*/
				if(indentation%master_state.indent_unit != 0){
					printf("IndentError: inconsistent indentation (line num %d)\n", line_num);

					return_value->value = 0;
					return_value->t  = 0;

					break;	
				}
				if(indentation/master_state.indent_unit < master_state.block_level){
					block_ended = 1;
				}
				temp_lastindent = master_state.last_indent;
				master_state.last_indent = indentation;
				// printf("indentation %d indent_unit %d last_indent %d\n", indentation, master_state.indent_unit, master_state.last_indent);
			} 
			if (tokens[i].ttype != TOKEN_WHITESPACE && i == 0){
				/* set last indent if we wouldn't otherwise */
				block_ended = 1;
				temp_lastindent = master_state.last_indent;
				master_state.last_indent = 0;
			}
		}
		if(i==0){temp_lastindent = master_state.last_indent;}
		if(i == 0 || block_ended){
			/* 2 possibilities: 
			* - correct end of block
			* - end of block too early */
			if(master_state.indent_unit == 0 || temp_lastindent/master_state.indent_unit < master_state.block_level){
				/* unindent too early */
				printf("IndentError: need body to block (line num %d)\n", line_num);

				return_value->value = 0;
				return_value->t  = 0;

				return 0;
			} else {

				master_state.running_block = 1;
				if(master_state.block_types[master_state.block_level] == BLOCK_IF){
					run_if(tokens, return_value, line_num);
				}
				
				if(master_state.block_types[master_state.block_level] == BLOCK_WHILE){
					run_while(tokens, return_value, line_num);
				}
				master_state.block_level-=1;
			}
			parse_tokens(tokens, return_value, line_num);
		}
	}
	
	if(block != BLOCK_NONE && strcmp(tokens[i-1].identifier, ":")){
		/* TODO: don't do this */
		printf("SyntaxError: Invalid block syntax (line num %d)\n", line_num);
		
		return_value->value = 0;
		return_value->t  = 0;

		return 0 ;	
	}	

}


/* function to tokenize and parse. This is to be called in interpreter */
int parse(char* line, variable* return_value, int line_num, int is_newline) {
	token*       tokens;
	static int   indentation_unit = 0;
	static int   last_indentation = 0;
	int          indentation      = 0;

	if(is_newline){
		/* copy line for later use */
		master_state.lines[line_num] = malloc(MAX_LINE_LENGTH);
		strcpy(master_state.lines[line_num], line);
		master_state.line_count = line_num;
		for(int i = 0; i < line_num; i++){
			if(master_state.lines[i] == NULL){
				master_state.lines[i] = malloc(MAX_LINE_LENGTH);
			}
		}
	}
	
	if(tokenize(line, &tokens)){
		parse_tokens(tokens, return_value, line_num);
		for(int i = 0; tokens[i].ttype != TOKEN_END; i++){
			free(tokens[i].identifier);
		}
		free(tokens);
	} else{
		return 0;
	}
	return 1;
}
