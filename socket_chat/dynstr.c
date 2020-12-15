#include "dynstr.h"
#include <stdio.h>
#include <malloc/_malloc.h>

void init_str(struct _string* str){
    str->k_mem = 16;
    str->size = 0;
    str->p = (char*) malloc(sizeof(char) * str->k_mem);
    str->p[0] = '\0';
}

void add_str_char(struct _string* str, char ch){
    if(ch != '\0'){
        if (((str->size) + 1) >= str->k_mem){
            char* new_p = (char*) malloc(sizeof(char) * (str->k_mem * 2));

            for(int i = 0; str->p[i] != '\0'; i++){
                new_p[i] = str->p[i];
            }

            free(str->p);

            str->p = new_p;
            str->k_mem *= 2;
        }

        str->p[str->size] = ch;
        str->size++;
        str->p[str->size] = '\0';
    }
}

void dispose_str(struct _string* str){
    free(str->p);
}

void delete_first(struct _string* string){
    if(string->size != 0){
        int i = 1;
        while (string->p[i] != '\0') {
            string->p[i - 1] = string->p[i];
            i++;
        }
        string->p[i - 1] = string->p[i];
    }
    string->size--;
}
