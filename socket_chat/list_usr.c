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
        (*list)->ready_to_send = false;
        (*list)->init_mes = false;
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
        p->next->init_mes = false;
        p->next->index_name = 0;
        p->next->ready_to_send = false;
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

void deletelem(list_usr* list, int elem){
    if(*list != NULL){
        // обработка первого элемента
        list_usr temp;
        if((*list)->val == elem){
            temp = (*list)->next;
            if((*list)->init_mes)
                dispose_srt(&(*list)->message);
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
            if(cur->next->init_mes)
                dispose_srt(&(cur->next->message));
            free(cur->next);
            cur->next = temp;
        }
    }
}

void clear_message(list_usr user){
    dispose_srt(&(user->message));
    user->init_mes = false;
    user->ready_to_send = false;
}
