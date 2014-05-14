#include "list.h"

#include <stdlib.h>
#include <string.h>

void list_init(struct list *list)
{
    memset(list, 0, sizeof(*list));
}

struct list *list_new(void *data)
{
    struct list *list = calloc(1, sizeof(*list));
    list->data = data;
    return list;
}

struct list *list_append(struct list *list, void *data)
{
    if (list == NULL)
    {
        return list_new(data);
    }

    struct list *last = list_get_last(list);
    list = list_new(data);
    last->next = list;

    return list;
}

struct list *list_get_last(struct list *list)
{
    while (list && list->next)
        list = list->next;
    return list;
}

struct list *list_pop_front(struct list **list)
{
    struct list *popped = *list;

    if (!popped)
        return NULL;

    *list = popped->next;
    popped->next = NULL;

    return popped;
}

void list_free(struct list *list, list_free_cb free_cb, void *userdata)
{
    struct list *popped;

    while ((popped = list_pop_front(&list)) != NULL)
    {
        if (free_cb)
            free_cb(popped->data, userdata);
        free(popped);
    }
}

#ifdef BUILD_TEST

#include "test_util.h"

TEST(list_init)
{

}

TEST(list_new)
{

}

TEST(list_append)
{

}

TEST(list_get_last)
{

}

TEST(list_pop_front)
{

}

TEST(list_free)
{

}


#endif
