#include <stdlib.h>
#include "tools.h"

/**
 **
 ** list - a dynamic array of pointers
 **
 ** LIST *make_list(count, ptr)        - make a new list using existing data
 ** LIST *new_list()                   - make a new empty list
 ** void  list_add(list, ptr)          - add data to an existing list
 ** int   list_count(list)             - return number of items in list
 ** void *list_data(list)              - return data in list
 ** void list_replace_data(l1, l2)     - replace l1->data with l2->data
 **
 ** Also usable with C++ like syntax:
 **
 ** list = new_list();                - create a new list
 ** list->add(list, ptr);             - add an item to a list
 ** list->data();                     - get pointer to data in list
 ** list->count();                    - get number of elements in list
 ** list->replace();                  - see above
 **
 ** When you are done adding stuff, grab your array of pointers and
 ** just free the list object:
 **
 ** 	free(list);
 **
 ** Note, this does not free list->data, just the LIST structure.  You will
 ** probably want to have grabbed the list data before freeing list:
 **
 ** 	list->data(list);
 **/

void *list_ptr(LIST * list, int whence);
void **list_data(LIST * list);
void *list_add(LIST * list, void *ptr);

LIST *
make_list(int count, void **ptr)
{
    LIST *new = calloc(1, sizeof(LIST));

    new->number = count;
    new->ptr = ptr;
    /**
     ** fill in pointers to functions.
     **/
    new->add = list_add;
    new->data = list_data;
    new->count = list_count;

    return (new);
}

LIST *
new_list()
{
    return (make_list(0, NULL));
}

void *
list_add(LIST * list, void *ptr)
{
    if (list->ptr == NULL)
        list->ptr = malloc(sizeof(void *));
    else
        list->ptr = realloc(list->ptr, sizeof(void *) * (list->number + 1));

    list->ptr[list->number] = ptr;
    list->number++;

    return (list->ptr[list->number - 1]);
}

void *
list_copy(LIST * list, void *ptr, int size)
{
    return (list_add(list, (void *)memdup(ptr, size)));
}

int
list_count(LIST * list)
{
    return (list->number);
}

void **
list_data(LIST * list)
{
    if (list->number != 0)
        return (list->ptr);
    else
        return (NULL);
}

void
list_free(LIST * list)
{
	 if (list->ptr != NULL)
        free(list->ptr);
    free(list);
}

/* USE WITH CAUTION */
void
list_kill(LIST *list)
{
	while(list->number){
		(list->number) --;
		if (((void **)list->ptr)[list->number]){
			free(((void **)list->ptr)[list->number]);
		}
	}
	list_free(list);
}

/* USE WITH CAUTION */
void
list_empty(LIST *list)
{
  free(list->ptr);
  list->number = 0;
}


void *
list_ptr(LIST * list, int whence)
{
    if (whence >= list->number)
        return (NULL);
    if (whence >= 0) {
        return (list->ptr[whence]);
    }
    /**
     ** whence == -1, return last.
     **/
    if (whence < 0 && (-whence) < list->number) {
        return (list->ptr[list->number + whence]);
    }

	 return NULL;
}

/**
 ** list_replace() - special purpose function to copy l2 into l1
 **/
void
list_replace(LIST *l1, LIST *l2)
{
    free(l1->ptr);
    l1->ptr = l2->ptr;
    l1->number = l2->number;
}

void
list_merge(LIST *l1, LIST *l2)
{
	int i = l1->number + l2->number;

    if (l1->ptr == NULL)
        l1->ptr = calloc(i, sizeof(void *));
    else
        l1->ptr = realloc(l1->ptr, sizeof(void *) * (i + 1));

	memcpy(&((void **)l1->ptr)[l1->number], l2->ptr, l2->number*sizeof(void *));
	l1->number = i;
}

/*****************************************************************************/
/* QUEUE - dynamically allocated linked list                                 */
/*                                                                           */
/* QUEUE *new_queue(data)                  - create a new queue              */
/* QUEUE *end_of_queue(queue)              - return last node in queue       */
/* QUEUE *add_queue_head(&queue, data)     - add data to front of queue      */
/* QUEUE *add_queue_tail(&queue, data)     - add data to end of queue        */
/* void remove_from_queue(&queue, data)    - remove node containing 'data'   */
/*                                                                           */
/*****************************************************************************/

QUEUE *
new_queue(void *data)
{
    QUEUE *new = calloc(1, sizeof(QUEUE));
    new->data = data;
    return (new);
}

QUEUE *
end_of_queue(QUEUE * queue)
{
    while (queue && queue->next)
        queue = queue->next;
    return (queue);
}

void
add_queue_head(QUEUE ** l, void *data)
{
    QUEUE *new;
    new = new_queue(data);
    new->next = *l;
    *l = new;
}

void
add_queue_tail(QUEUE ** l, void *data)
{
    if (*l)
        end_of_queue(*l)->next = new_queue(data);
    else
        *l = new_queue(data);
}

void
remove_from_queue(QUEUE ** l, void *data)
{
    QUEUE *t;
    if ((*l)->data == data) {
        t = (*l);
        (*l) = (*l)->next;
        free(t);
    } else {
        while ((*l)->next && (*l)->next->data != data)
            *l = (*l)->next;
        if ((*l)->next->data == data) {
            t = (*l)->next;
            (*l)->next = (*l)->next->next;
            free(t);
        }
    }
}

QUEUE *
next_in_queue(QUEUE * l)
{
    return (l->next);
}

void *
queue_data(QUEUE * l)
{
    return (l->data);
}


