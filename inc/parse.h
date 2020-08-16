/* tells what format the current line is */
typedef enum {
	TOKEN_VAR = 0, 
	TOKEN_OP,
	TOKEN_WHITESPACE, /* keep simple for now */
	TOKEN_END
} token_type;

/* tells type of variable */
typedef enum {
	TYPE_NUL = 0,
	TYPE_INT8 = 8, TYPE_INT16 = 16,
	TYPE_INT32 = 32, TYPE_INT64 = 64,
	TYPE_POINTER = sizeof(void*)
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

/* generic variable structure */
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

/* current state of interpreter */
typedef struct {
	variable*  vars;
	int var_num;
	variable* cons;
	int con_num;
	int* block_line_num;
	block_type* block_types;
	int line_count;
	int block_level;
	int can_unindent;
	int last_indent;
	int indent_unit;
} state;

/* setup for parser */
int   start_parser();

/* actual parse function */
int  parse(char* line, variable* return_value, int line_num);

/* "stack" simulation */
int   push_back_master_state();

/* stop parser, clean memory */
int   stop_parser();