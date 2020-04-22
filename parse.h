typedef enum {
	OPERATION = 0, FUNCTION,
	DEFINITION, DATATYPE
} format;

typedef enum {
	INT8 = 8, INT16 = 16,
	INT32 = 32, POINTER = sizeof(void*)
} type;

typedef struct {
	char* identifier;
	void* location;
} variable;

typedef struct {
	char* identifier;
	void* value;
} constant;

typedef struct {
	variable*  vars;
	constant* cons;
} state;

typedef struct {
	char** keywords;
	int keyword_num;
	char** consts;
	int const_num;
	char** vars;
	int var_num;
	format f;
	char   inited;
} line_structure;

state  master_state;
state  temp_state;
state* state_stack;
int current_state;


int   start_parser();

void* parse(char* line);


int   push_back_master_state();