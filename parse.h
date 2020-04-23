typedef enum {
	OPERATION = 0, FUNCTION,
	DEFINITION, DATATYPE
} format;

typedef enum {
	NUL = 0,
	INT8 = 8, INT16 = 16,
	INT32 = 32, INT64 = 64,
	POINTER = sizeof(void*)
} type;

typedef struct {
	char* identifier;
	void* value;
	type t;
} variable;

typedef struct {
	char** keywords;
	int keyword_num;
	format f;
	char   inited;
} line_structure;

typedef struct {
	variable*  vars;
	int var_num;
	variable* cons;
	int cons_num;
} state;

int   start_parser();

void  parse(char* line, variable* return_value);

int   push_back_master_state();