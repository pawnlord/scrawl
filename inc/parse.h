/* tells what format the current line is */
typedef enum {
	OPERATION = 0, FUNCTION,
	DEFINITION, DATATYPE
} format;

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

/* tells us the structure of line (keywords, format) */
typedef struct {
	char** keywords;
	int keyword_num;
	format f;
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
} state;

/* setup for parser */
int   start_parser();

/* actual parse function */
void  parse(char* line, variable* return_value, int execute, int stop_at_symbol, int line_num);

/* "stack" simulation */
int   push_back_master_state();

/* stop parser, clean memory */
int   stop_parser();