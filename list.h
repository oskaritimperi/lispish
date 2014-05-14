#ifndef LIST_H
#define LIST_H

struct list
{
    void *data;
    struct list *next;
};

typedef void (*list_free_cb)(void *data, void *userdata);

void list_init(struct list *list);
struct list *list_new(void *data);
struct list *list_append(struct list *list, void *data);
struct list *list_get_last(struct list *list);
struct list *list_pop_front(struct list **list);
void list_free(struct list *list, list_free_cb free_cb, void *userdata);

#endif
