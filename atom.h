#ifndef ATOM_H
#define ATOM_H

#include <sys/queue.h>

#define ATOM_TYPE(ATOM) ((ATOM)->type)

#define IS_INT(ATOM) ((ATOM_TYPE(ATOM)) == ATOM_INT)
#define IS_STR(ATOM) ((ATOM_TYPE(ATOM)) == ATOM_STR)
#define IS_SYM(ATOM) ((ATOM_TYPE(ATOM)) == ATOM_SYMBOL)
#define IS_LIST(ATOM) ((ATOM_TYPE(ATOM)) == ATOM_LIST)
#define IS_ATOM(ATOM) (!(IS_LIST(ATOM)))

#define IS_TRUE(ATOM) (ATOM_TYPE(ATOM) == ATOM_TRUE)
#define IS_FALSE(ATOM) (ATOM_TYPE(ATOM) == ATOM_FALSE)

#define IS_NIL(ATOM) (ATOM_TYPE(ATOM) == ATOM_NIL)

#define IS_CLOSURE(ATOM) (ATOM_TYPE(ATOM) == ATOM_CLOSURE)

#define CAR(LIST) (LIST_FIRST(LIST))
#define CDR(LIST) ((LIST) != NULL ? LIST_NEXT((LIST), entries) : NULL)
#define CDDR(LIST) CDR(CDR(LIST))

enum
{
    ATOM_NIL,
    ATOM_INT,
    ATOM_STR,
    ATOM_SYMBOL,
    ATOM_LIST,
    ATOM_TRUE,
    ATOM_FALSE,
    ATOM_CLOSURE
};

struct atom;
struct env;

struct closure
{
    struct env *env;
    struct atom *params;
    struct atom *body;
};

LIST_HEAD(list, atom);

struct atom
{
    char type;

    union
    {
        long l;
        struct
        {
            char *str;
            int len;
        } str;
        struct list *list;
        struct closure closure;
    };

    LIST_ENTRY(atom) entries;
};

struct atom *atom_new(char type);
struct atom *atom_new_int(long l);
struct atom *atom_new_str(const char *str, int len);
struct atom *atom_new_sym(const char *sym, int len);
struct atom *atom_new_list(struct list *list);
struct atom *atom_new_list_empty();
struct atom *atom_new_closure(struct atom *params, struct atom *body,
    struct env *env);
struct atom *atom_clone();

void print_atom(struct atom *atom, int level);

struct atom *atom_list_append(struct atom *list, int count, ...);
int atom_list_length(struct atom *list);

extern struct atom true_atom;
extern struct atom false_atom;
extern struct atom nil_atom;

#endif
