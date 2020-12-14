#include <stdbool.h>
#include "dynstr.h"

struct node{
    int val;
    struct node* next;
    char name[18];
    int index_name;
    struct _string message;
    int is_named;
    bool init_mes;
    bool ready_to_send;
};

typedef struct node* list_usr;

void add_list(struct node** list, int x);
void print_list(struct node* list);
int len_list(struct node* list);
int search_elem_list(struct node* list, int elem);
void dispose_list(struct node** list);
bool is_empty_list(list_usr list);
void deletelem(list_usr* list, int elem);
void clear_message(list_usr user);
