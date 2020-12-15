#pragma once

struct _string{
    char* p;
    int size;
    int k_mem;
};

void init_str(struct _string* str);
void add_str_char(struct _string* str, char ch);
void dispose_str(struct _string* str);
void delete_first(struct _string* string);
