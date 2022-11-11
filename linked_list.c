#include "linked_list.h"
#include <stdlib.h>

struct linked_list_t* ll_create(){
    struct linked_list_t *lista = calloc(sizeof(struct linked_list_t), 1);
    if(lista == NULL){
        return NULL;
    }
    lista->head = NULL;
    lista->tail = NULL;
    return lista;
}

int ll_push_back(struct linked_list_t* ll, int pos_x, int pos_y, int value){
    if(ll == NULL){
        return 1;
    }
    struct node_t *new = calloc(sizeof(struct node_t), 1);
    if(new == NULL){
        return 2;
    }
    new->pos_x = pos_x;
    new->pos_y = pos_y;
    new->value = value;
    new->next = NULL;
    if(ll->tail == NULL || ll->head == NULL){
        ll->tail= new;
        ll->head = new;
    }
    else {
        ll->tail->next = new;
        ll->tail = new;
    }

    return 0;
}
int ll_size(const struct linked_list_t* ll){
    if(ll == NULL){
        return -1;
    }
    int size = 0;
    if(ll->head == NULL || ll->tail == NULL){
        return 0;
    }
    struct node_t *temp = ll->head;
    while(temp->next != NULL){
        size++;
        temp = temp->next;
    }
    return size+1;
}
int in_list(struct linked_list_t* ll, int pos_x, int pos_y){
    if(ll == NULL || ll->head == NULL || ll->tail == NULL){
        return -1;
    }

    struct node_t *temp = ll->head;
    int idx = 0;
    while(temp != NULL){
        if(temp->pos_y == pos_y && temp->pos_x == pos_x){
            int val = temp->value;
            ll_remove(ll, idx);
            return val;
        }
        idx++;
        temp = temp->next;
    }

    return -1;
}
int ll_remove(struct linked_list_t* ll, unsigned int index){
    if(ll == NULL || ll->head == NULL || ll->tail == NULL || (int)index < 0){
        return 1;
    }

    int size = ll_size(ll);

    if(size < (int)index || size == 0){
        return 1;
    }

    if(index == 0){
        struct node_t *temp = ll->head->next;
        free(ll->head);
        ll->head = temp;
        return 0;
    }
    if((int)index == size-1){
        struct node_t *temp = ll->head;
        for(int i = 0; i<size-2; i++){
            temp = temp->next;
        }
        free(ll->tail);
        temp->next = NULL;
        ll->tail = temp;
        return 0;
    }

    struct node_t *before = ll->head;
    struct node_t *after = ll->head;
    struct node_t *to_delete = ll->head;
    int counter = 0;
    while(counter < (int)(index-1)){
        counter++;
        before = before->next;
    }
    counter = 0;
    while(counter != (int)index){
        counter++;
        to_delete = to_delete->next;
    }
    counter = 0;
    while(counter != (int)(index+1)){
        counter++;
        after = after->next;
    }
    free(to_delete);
    before->next = after;
    return 0;
}
