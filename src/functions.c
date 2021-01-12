#include "../inc/functions.h"
#include <stdio.h>

void make_function(function_t* ft, function f, variable** params, char* identifier){
    ft->f = f;
    ft->params = params;
    ft->identifier = identifier;
}

variable funcprint(variable** params_ref){
    variable* params = *params_ref;
    for(int i = 0; params[i].t != TYPE_NUL; i++){
        if(params[i].t == TYPE_INT16 || params[i].t == TYPE_INT32 || params[i].t == TYPE_INT64 || params[i].t == TYPE_INT8 ){
            printf("%d\n", params[i].value);
        } else if (params[i].t == TYPE_BOOL){
            printf("%s\n", params[i].value>0?"true":"false");
        } else if (params[i].t == TYPE_STRING){
            printf("%s\n", (char*)params[i].value);
        }
    }
    variable v;
    v.identifier = "";
    v.t = TYPE_NUL;
    v.value = 0;
    return v;
}