#ifndef ATOM_H
#define ATOM_H

#define LIST_GET_ATOM(LIST) ((struct atom *) (LIST)->data)

#define ATOM_TYPE(LIST) ((LIST_GET_ATOM(LIST))->type)
#define IS_INT(LIST) ((ATOM_TYPE(LIST)) == ATOM_INT)
#define IS_STR(LIST) ((ATOM_TYPE(LIST)) == ATOM_STR)
#define IS_SYM(LIST) ((ATOM_TYPE(LIST)) == ATOM_SYMBOL)
#define IS_LIST(LIST) ((ATOM_TYPE(LIST)) == ATOM_LIST)

#define IS_TRUE(LIST) (LIST_GET_ATOM(LIST) == &true_atom)
#define IS_FALSE(LIST) (LIST_GET_ATOM(LIST) == &false_atom)
#define IS_NIL(LIST) (LIST_GET_ATOM(LIST) == &nil_atom)

#define CAR(LIST) LIST

#define CDR(LIST) ((LIST) != NULL ? (LIST)->next : NULL)
#define CDDR(LIST) CDR(CDR(LIST))

enum
{
    ATOM_NIL,
    ATOM_INT,
    ATOM_STR,
    ATOM_SYMBOL,
    ATOM_LIST
};

struct atom
{
    union
    {
        long l;
        struct
        {
            char *str;
            int len;
        } str;
        struct list *list;
    };
    char type;
};

struct list;

struct atom *atom_new(char type);
struct atom *atom_new_int(long l);
struct atom *atom_new_str(const char *str, int len);
struct atom *atom_new_sym(const char *sym, int len);
struct atom *atom_new_list(struct list *list);

extern struct atom true_atom;
extern struct atom false_atom;
extern struct atom nil_atom;

#endif
