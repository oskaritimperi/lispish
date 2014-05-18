#include "atom.h"

#include <stdlib.h>
#include <stdio.h>
#include <string.h>

struct atom true_atom = { ATOM_TRUE };
struct atom false_atom = { ATOM_FALSE };
struct atom nil_atom = { ATOM_NIL } ;

__attribute__((constructor))
static void setup_builtin_atoms()
{
    nil_atom.list = calloc(1, sizeof(*nil_atom.list));
    LIST_INIT(nil_atom.list);
}

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

struct atom *atom_new_list_empty()
{
    struct list *list = calloc(1, sizeof(*list));
    LIST_INIT(list);
    return atom_new_list(list);
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
        struct atom *elem, *last;

        struct list *list_clone = calloc(1, sizeof(*list_clone));
        LIST_INIT(list_clone);

        LIST_FOREACH(elem, atom->list, entries)
        {
            struct atom *a_clone = atom_clone(elem);

            if (LIST_EMPTY(list_clone))
                LIST_INSERT_HEAD(list_clone, a_clone, entries);
            else
                LIST_INSERT_AFTER(last, a_clone, entries);

            last = a_clone;
        }

        return atom_new_list(list_clone);
    }
    }

    return NULL;
}

void print_atom(struct atom *atom, int level)
{
    switch (ATOM_TYPE(atom))
    {
    case ATOM_TRUE: printf("#t"); break;
    case ATOM_FALSE: printf("#f"); break;
    case ATOM_NIL: printf("nil"); break;

    case ATOM_SYMBOL:
        printf("%.*s", atom->str.len, atom->str.str);
        break;

    case ATOM_STR:
        printf("\"%.*s\"", atom->str.len, atom->str.str);
        break;

    case ATOM_INT:
        printf("%ld", atom->l);
        break;

    case ATOM_LIST:
    {
        printf("(");
        struct atom *elem;
        LIST_FOREACH(elem, atom->list, entries)
        {
            print_atom(elem, level+1);
            if (LIST_NEXT(elem, entries))
                printf(" ");
        }
        printf(")");
    }
    }

    if (level == 0)
        printf("\n");
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
