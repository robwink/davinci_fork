#ifndef TOOLS_H
#define TOOLS_H


/**
 **
 **/
struct tag_queue {
	void *data;
	struct tag_queue *next;
};

typedef struct tag_queue QUEUE;

QUEUE *new_queue(void *data);
QUEUE *end_of_queue(QUEUE *queue);
void add_queue_head(QUEUE **queue, void *data);
void add_queue_tail(QUEUE **queue, void *data);
void remove_from_queue(QUEUE **queue, void *data);
QUEUE * next_in_queue(QUEUE *queue);
void * queue_data(QUEUE *queue);

typedef struct _list LIST;
typedef void *(*vfptr)(LIST *);
struct _list {
	int number;
	void **ptr;
	void *(*add)(LIST *, void *);		/* add element */
	void **(*data)(LIST *);			/* get data */
	int (*count)(LIST *);			/* get count */
};

void *list_add(LIST *list, void *ptr);
int list_count(LIST *list);
void **list_data(LIST *list);
LIST *make_list_for_data(int count, void **ptr);
LIST *new_list();
void list_merge(LIST *l1, LIST *l2);

/* Free memory allocated by list structure itself. Does not free
	the elements */
void list_free(LIST *l);

/* Do what list_free() does plus run free() on each of the
	list elements. Good for lists of char*'s */
void list_kill(LIST *l);

#endif /* TOOLS_H */
