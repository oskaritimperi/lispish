#include "atom.h"
#include "list.h"

#include <stdlib.h>
#include <string.h>

struct atom true_atom = { ATOM_TRUE };
struct atom false_atom = { ATOM_FALSE };
struct atom nil_atom = { ATOM_NIL } ;

struct atom *atom_new(char type)
{
    struct atom *atom = calloc(1, sizeof(*atom));
    atom->type = type;
    return atom;
}

struct atom *atom_new_int(long l)
{
    struct atom *atom = atom_new(ATOM_INT);
    atom->l = l;
    return atom;
}

struct atom *atom_new_str(const char *str, int len)
{
    struct atom *atom = atom_new(ATOM_STR);
    atom->str.str = strndup(str, len);
    atom->str.len = len;
    return atom;
}

struct atom *atom_new_sym(const char *sym, int len)
{
    struct atom *atom = atom_new_str(sym, len);
    atom->type = ATOM_SYMBOL;
    return atom;
}

struct atom *atom_new_list(struct list *list)
{
    struct atom *atom = atom_new(ATOM_LIST);
    atom->list = list;
    return atom;
}

struct atom *atom_clone(struct atom *atom)
{
    switch (atom->type)
    {
    case ATOM_NIL:
    case ATOM_TRUE:
    case ATOM_FALSE:
        return atom;

    case ATOM_INT:
        return atom_new_int(atom->l);

    case ATOM_STR:
        return atom_new_str(atom->str.str, atom->str.len);

    case ATOM_SYMBOL:
        return atom_new_sym(atom->str.str, atom->str.len);

    case ATOM_LIST:
    {
        struct list *list = atom->list;
        struct list *clone = NULL, *last = NULL;

        while (list)
        {
            struct atom *a = LIST_GET_ATOM(list);
            struct atom *b = atom_clone(a);
            last = list_append(last, b);
            list = list->next;
            if (!clone)
                clone = last;
        }

        return atom_new_list(clone);
    }
    }

    return NULL;
}

#ifdef BUILD_TEST

#include "test_util.h"

TEST(atom_new)
{
    struct atom *atom = atom_new(ATOM_STR);
    ASSERT_TRUE(atom != NULL);
    ASSERT_EQ(ATOM_STR, atom->type);
}

TEST(atom_new_int)
{
    struct atom *atom = atom_new_int(42);
    ASSERT_TRUE(atom != NULL);
    ASSERT_EQ(42, atom->l);
}

TEST(atom_new_str)
{
    struct atom *atom = atom_new_str("foobar", 6);
    ASSERT_TRUE(atom != NULL);
    ASSERT_STREQ("foobar", atom->str.str);

    atom = atom_new_str("foobar", 3);
    ASSERT_TRUE(atom != NULL);
    ASSERT_STREQ("foo", atom->str.str);

    ASSERT_EQ(ATOM_STR, atom->type);
}

TEST(atom_new_sym)
{
    struct atom *atom = atom_new_sym("foobar", 6);
    ASSERT_TRUE(atom != NULL);
    ASSERT_STREQ("foobar", atom->str.str);

    atom = atom_new_sym("foobar", 3);
    ASSERT_TRUE(atom != NULL);
    ASSERT_STREQ("foo", atom->str.str);

    ASSERT_EQ(ATOM_SYMBOL, atom->type);
}

TEST(atom_new_list)
{
    struct list list;
    struct atom *atom = atom_new_list(&list);
    ASSERT_TRUE(atom != NULL);
    ASSERT_EQ(&list, atom->list);
    ASSERT_EQ(ATOM_LIST, atom->type);
}

#endif
