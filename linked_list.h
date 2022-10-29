#ifndef SECOND_TRY_LINKED_LIST_H
#define SECOND_TRY_LINKED_LIST_H

struct node_t
{
    int pos_x;
    int pos_y;
    int value;
    struct node_t *next;
};

typedef struct linked_list_t
{
    struct node_t *head;
    struct node_t *tail;
}linked_list;

struct linked_list_t* ll_create();
int ll_push_back(struct linked_list_t* ll, int pos_x, int pos_y, int value);
int in_list(struct linked_list_t* ll, int pos_x, int pos_y);
int ll_remove(struct linked_list_t* ll, unsigned int index);

#endif //SECOND_TRY_LINKED_LIST_H
