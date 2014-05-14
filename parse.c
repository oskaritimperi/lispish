#include "parse.h"
#include "tokens.h"
#include "list.h"
#include "atom.h"

#include <stdio.h>
#include <stdlib.h>
#include <string.h>
#include <ctype.h>

void print_list(struct list *list, int level)
{
    while (list)
    {
        struct atom *atom = LIST_GET_ATOM(list);

        if (IS_TRUE(list))
        {
            printf("#t");
        }
        else if (IS_FALSE(list))
        {
            printf("#f");
        }
        else if (IS_NIL(list))
        {
            printf("nil");
        }
        else
        {
            switch (atom->type)
            {
                case ATOM_SYMBOL:
                    printf("%.*s", atom->str.len, atom->str.str);
                    break;

                case ATOM_LIST:
                    printf("(");
                    print_list(atom->list, level+1);
                    printf(")");
                    break;

                case ATOM_STR:
                    printf("\"%.*s\"", atom->str.len, atom->str.str);
                    break;

                case ATOM_INT:
                    printf("%ld", atom->l);
            }
        }

        if (list->next)
            printf(" ");

        list = list->next;
    }

    if (level == 0)
        printf("\n");
}

int parse_next(const char *src, int *pos, struct atom **result)
{
    struct token token;
    int rc;

    if ((rc = get_next_token(src, pos, &token)) > 0)
    {
        switch (token.type)
        {
            case TOKEN_INT:
                *result = atom_new_int(strtol(token.s, NULL, 10));
                break;

            case TOKEN_STR:
                *result = atom_new_str(token.s, token.len);
                break;

            case TOKEN_SYMBOL:
                if (strncmp(token.s, "#t", 2) == 0)
                    *result = &true_atom;
                else if (strncmp(token.s, "#f", 2) == 0)
                    *result = &false_atom;
                else
                    *result = atom_new_sym(token.s, token.len);
                break;

            case TOKEN_LPAREN:
            {
                struct list *l = parse(src, pos);
                *result = atom_new_list(l);
                break;
            }

            case TOKEN_RPAREN:
                return -2;

            case TOKEN_QUOTE:
            {
                struct atom *quoted = NULL;

                parse_next(src, pos, &quoted);

                struct list *qlist = list_append(NULL,
                    atom_new_sym("quote", 5));

                list_append(qlist, quoted);

                *result = atom_new_list(qlist);

                break;
            }
        }
    }

    return rc;
}

struct list *parse(const char *src, int *pos)
{
    int rc;
    struct atom *atom;
    struct list root, *last = &root;

    list_init(last);

    while ((rc = parse_next(src, pos, &atom)))
    {
        if (rc < 0)
            break;

        last = list_append(last, atom);
    }

    if (rc < 0 && rc != -2)
        return NULL;

    return root.next;
}

#ifdef BUILD_TEST

#include "test_util.h"

static const char *test_src_fact =
    "(define fact\n"
    "    ;; Factorial function\n"
    "    (lambda (n)\n"
    "        (if (eq n 0)\n"
    "            1 ; Factorial of 0 is 1\n"
    "            (* n (fact (- n 1))))))\n"
    "(fact 5) ;; this should evaluate to 120\n";

#define ASSERT_SYM(LIST, SYM) \
    ASSERT_TRUE(IS_SYM(LIST)); \
    ASSERT_STREQ(SYM, LIST_GET_ATOM(LIST)->str.str)

TEST(test_parse)
{
    int pos = 0;
    struct list *list;

    list = parse(test_src_fact, &pos);

    ASSERT_TRUE(list != NULL);

    ASSERT_TRUE(IS_LIST(list));

    list = LIST_GET_ATOM(list)->list;

    ASSERT_TRUE(list != NULL);
    ASSERT_SYM(list, "define");

    list = list->next;

    ASSERT_TRUE(list != NULL);
    ASSERT_SYM(list, "fact");

    list = list->next;

    ASSERT_TRUE(list != NULL);
}

#endif
