/*
	All important datastructures
	Put them all in one place so each can reference eachother
*/
#ifndef DATA_INFORMATION_H
#define DATA_INFORMATION_H
/* tells what format the current token is */
typedef enum {
	TOKEN_VAR = 0, 
	TOKEN_OP,
	TOKEN_WHITESPACE, /* keep simple for now */
	TOKEN_END
} token_type;

/* tells type of variable */
typedef enum {
	TYPE_NUL = 0, TYPE_BOOL = 1,
	TYPE_INT8 = 2, TYPE_INT16 = 3,
	TYPE_INT32 = 4, TYPE_INT64 = 5,
	TYPE_ARRAY = 6, TYPE_STRING = 7
} type;

/* tells type of block */
typedef enum {
	BLOCK_NONE = 0, /* none. just for scope */
	BLOCK_IF,
	BLOCK_WHILE
	/* Future Types: BLOCK_FOR, BLOCK_FUNCTION, BLOCK_HANDLER*/ 
} block_type;

/* generic variable structure */
typedef struct {
	char* identifier;
	void* value;
	type t;
} variable;

/* generic token structure */
typedef struct {
	char* identifier;
	token_type ttype;

} token;

/* tells us the structure of line (keywords, format) */
typedef struct {
	char** keywords;
	int keyword_num;
	char   inited;
} line_structure;

typedef variable (*function)(variable** params);

typedef struct{
    function f;
    char* identifier;
    variable** params;
} function_t;

/* current state of interpreter */
typedef struct {
	variable*  vars;
	int var_num;
	variable* cons;
	int con_num;
	int* block_line_num;
	block_type* block_types;
	int line_count;
	char** lines;
	int block_level;
	int can_unindent;
	int last_indent;
	int indent_unit;
	int running_block;
	int stop_comparison;
	volatile int* exit_loop;
    function_t* functions;
} state;
#endif