#include "../inc/functions.h"
#include <stdio.h>
#include <stdlib.h>
#include <string.h>

void make_function(function_t* ft, function f, variable** params, char* identifier){
    ft->f = f;
    ft->params = params;
    ft->identifier = identifier;
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