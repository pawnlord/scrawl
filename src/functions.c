#include "../inc/functions.h"
#include <stdio.h>
#include <stdlib.h>
#include <string.h>
#include <stdarg.h>

void make_function(function_t* ft, function f, char* identifier){
    ft->f = f;
    ft->identifier = identifier;
}
type upgradetype(type t){
	if(t == TYPE_INT8 || t == TYPE_INT16 || t == TYPE_INT32 || t == TYPE_INT64 ){
		return TYPE_INT64;
	}
	return t;
}


int checkargs(variable* params, int count, ...){
    va_list ts;
    va_start(ts, count);
    int i;
    for(i = 0; i < count; i++){
        type t = va_arg(ts, type);
        if(upgradetype(params[i].t) != t && t != TYPE_UNKNOWN){
            return 0;
        }
    }
    if(params[i].t != TYPE_NUL){ return 0;}
    return 1;

}

variable funcprint(variable** params_ref){
    variable* params = *params_ref;
    static int is_newline = 1;
    for(int i = 0; params[i].t != TYPE_NUL; i++){
        if(params[i].t == TYPE_INT16 || params[i].t == TYPE_INT32 || params[i].t == TYPE_INT64 || params[i].t == TYPE_INT8 ){
            printf("%d ", params[i].value);
        } else if (params[i].t == TYPE_BOOL){
            printf("%s ", params[i].value>0?"true":"false");
        } else if (params[i].t == TYPE_STRING){
            printf("%s ", (char*)params[i].value);
        } else if (params[i].t == TYPE_ARRAY){
            variable* v = (variable*)params[i].value;
            printf("[");
            is_newline = 0;
            funcprint(&v);
            is_newline = 1;
            printf("]");
        }
    }
    if(is_newline){
        printf("\n"); /* End line */
    }
    variable v;
    v.identifier = "";
    v.t = TYPE_NUL;
    v.value = 0;
    return v;
}
variable functypeof(variable** params_ref){
    variable* params = *params_ref;
    for(int i = 0; params[i].t != TYPE_NUL; i++){
        variable v;
        v.identifier = malloc(10);
        sprintf(v.identifier, "%d", params[i].t);
        v.t = TYPE_INT16;
        v.value = (void*)params[i].t;
        return v;
    }
    variable v;
    v.identifier = "25";
    v.t = TYPE_INT16;
    v.value = (void*)25;
    return v;
}
variable funcsystem(variable** params_ref){
    variable* params = *params_ref;
    char* command = malloc(1000);
    for(int i = 0; i < 1000; i++){
        command[i] = 0;
    }
    for(int i = 0; params[i].t != TYPE_NUL; i++){
        if(params[i].t == TYPE_STRING){
            strcat(command, params[i].value);
            strcat(command, " ");
        }
    }
    system(command);
    free(command);
 
    variable v;
    v.identifier = "";
    v.t = TYPE_NUL;
    v.value = 0;
    return v;
}
variable funcarray(variable** params_ref){
    variable* params = *params_ref;
    int size = 0;
    for(int i = 0; params[i].t != TYPE_NUL; i++){
        size = (int)params[i].value;
    }
    variable v;
    v.identifier = "";
    v.t = TYPE_ARRAY;
    v.value = (void*)malloc(size * sizeof(variable));
    for(int i = 0; i < size; i++){
        ((variable*)v.value)[i].value = 0;
        ((variable*)v.value)[i].t = TYPE_INT64;
        ((variable*)v.value)[i].identifier = malloc(255);
        strcpy(((variable*)v.value)[i].identifier, "");
    }
    return v;
}

variable funcset(variable** params_ref){
    variable* params = *params_ref;
    int size = 0;
    checkargs(params, 2, TYPE_ARRAY, TYPE_INT64, TYPE_UNKNOWN);
    variable* arr = (variable*)params[0].value;
    arr[(int)(params[1].value)].value = params[2].value;
    arr[(int)(params[1].value)].t = params[2].t;
    
    variable v;
    v.identifier = "";
    v.t = TYPE_NUL;
    v.value = 0;
    return v;
}