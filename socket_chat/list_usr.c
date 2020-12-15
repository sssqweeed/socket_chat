#include "list_usr.h"
#include <stdlib.h>
#include <stdio.h>
#include <unistd.h>
#include "dynstr.h"

void add_list(struct node** list, int x){
    
    if (*list == NULL){
        *list = malloc(sizeof(struct node));
        (*list)->next = NULL;
        (*list)->val = x;
        (*list)->is_named = false;
        (*list)->is_init_buf = false;
        (*list)->index_name = 0;
    }
    else{
        struct node* p = *list;

        while (p->next != NULL){
            p = p->next;
        }

        p->next = malloc(sizeof(struct node));
        p->next->next = NULL;
        p->next->val = x;
        p->next->is_named = false;
        p->next->is_init_buf = false;
        p->next->index_name = 0;
        return;
    }
}

void print_list(struct node* list){
    printf("PRINT LIST:\n");
    while (list != NULL) {
        printf("ELEM: %d; NAME: [%s]; is_named: [%d]\n",
               list->val, list->name, list->is_named);
        
        list = list->next;
    }
    printf("END LIST\n");
}


int len_list(struct node* list){
    int k = 0;
    while (list != NULL) {
        k++;
        list = list->next;
    }
    return k;
}

int search_elem_list(struct node* list, int elem){
    int k = 0;
    while (list != NULL) {
        
        if(list->val == elem)
            break;
        k++;
        list = list->next;
        
    }
    return k;
}

void dispose_list(struct node** list){
    if((*list) == NULL){
        return;
    }
    else{
        dispose_list(&(*list)->next);
        free(*list);
        return;
    }
}

bool is_empty_list(list_usr list){
    return list == NULL;
}

void delete_elem(list_usr* list, int elem){
    if(*list != NULL){
        // обработка первого элемента
        list_usr temp;
        if((*list)->val == elem){
            temp = (*list)->next;
            if((*list)->is_init_buf)
                dispose_str(&(*list)->buffer);
            free((*list));
            *list = temp;
            
        }
        else{
            list_usr cur = *list;
            while(cur->next->val != elem){
                cur = cur->next;
                if(cur->next == NULL){
                    // не нашли элемента
                    return;
                }
            }
            temp = cur->next->next;
            if(cur->next->is_init_buf)
                dispose_str(&(cur->next->buffer));
            free(cur->next);
            cur->next = temp;
        }
    }
}

void clear_message(list_usr user){
    dispose_str(&(user->buffer));
    user->is_init_buf = false;
}
