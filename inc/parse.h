/* tells what format the current line is */
typedef enum {
	OPERATION = 0, FUNCTION,
	DEFINITION, DATATYPE
} format;

/* tells type of variable */
typedef enum {
	NUL = 0,
	INT8_e = 8, INT16_e = 16,
	INT32_e = 32, INT64_e = 64,
	POINTER = sizeof(void*)
} type;

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
} state;

/* setup for parser */
int   start_parser();

/* actual parse function */
void  parse(char* line, variable* return_value, int stop_at_symbol);

/* "stack" simulation */
int   push_back_master_state();

/* stop parser, clean memory */
int   stop_parser();