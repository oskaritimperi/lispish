#include "parse.h"
#include "tokens.h"
#include "atom.h"

#include <stdio.h>
#include <stdlib.h>
#include <string.h>
#include <ctype.h>

struct atom *parse_token(struct token *token)
{
    switch (token->type)
    {
    case TOKEN_INT:
        return atom_new_int(strtol(token->s, NULL, 10));

    case TOKEN_STR:
        return atom_new_str(token->s, token->len);

    case TOKEN_SYMBOL:
        if (strncmp(token->s, "#t", 2) == 0)
            return &true_atom;
        else if (strncmp(token->s, "#f", 2) == 0)
            return &false_atom;
        else
            return atom_new_sym(token->s, token->len);

    default:
        return NULL;
    }

    return NULL;
}

void parse_quote(const char *src, int *pos, struct atom **result)
{
    struct atom *value = parse(src, pos);
    struct atom *q = atom_new_sym("quote", 5);
    struct atom *form = atom_new_list_empty();

    LIST_INSERT_HEAD(form->list, q, entries);
    LIST_INSERT_AFTER(q, value, entries);

    *result = form;
}

int parse_list(const char *src, int *pos, struct atom **result)
{
    struct token token;
    int rc;
    struct list *list;
    struct atom *last = NULL;

    list = calloc(1, sizeof(*list));
    LIST_INIT(list);

    while ((rc = get_next_token(src, pos, &token)))
    {
        struct atom *atom;

        if (rc < 0)
            break;

        switch (token.type)
        {
        case TOKEN_LPAREN:
            if (parse_list(src, pos, &atom) < 0)
                return -1;
            break;

        case TOKEN_RPAREN:
            goto out;
            break;

        case TOKEN_QUOTE:
            parse_quote(src, pos, &atom);
            break;

        default:
            atom = parse_token(&token);
            break;
        }

        if (!last)
            LIST_INSERT_HEAD(list, atom, entries);
        else
            LIST_INSERT_AFTER(last, atom, entries);

        last = atom;
    }

out:

    if (rc < 0)
        return rc;

    if (LIST_EMPTY(list))
    {
        free(list);
        *result = &nil_atom;
        return 1;
    }

    *result = atom_new_list(list);
    return 1;
}

struct atom *parse(const char *src, int *pos)
{
    struct token token;
    struct atom *atom = NULL;
    int rc;

    rc = get_next_token(src, pos, &token);

    if (rc < 0)
        return NULL;

    switch (token.type)
    {
    case TOKEN_LPAREN:
        if (parse_list(src, pos, &atom) < 0)
            atom = NULL;
        break;

    case TOKEN_RPAREN:
        printf("syntax error: unexpected ')'\n");
        break;

    case TOKEN_QUOTE:
        parse_quote(src, pos, &atom);
        break;

    default:
        atom = parse_token(&token);
        break;
    }

    return atom;
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

#define ASSERT_SYM(ATOM, SYM) \
    ASSERT_TRUE(IS_SYM(ATOM)); \
    ASSERT_STREQ(SYM, (ATOM)->str.str)

TEST(test_parse)
{
    int pos = 0;
    struct atom *list;

    list = parse(test_src_fact, &pos);

    ASSERT_TRUE(list != NULL);
    ASSERT_TRUE(IS_LIST(list));

    struct atom *a = CAR(list->list);
    ASSERT_TRUE(a != NULL);
    ASSERT_SYM(a, "define");

    a = CDR(a);

    ASSERT_TRUE(a != NULL);
    ASSERT_SYM(a, "fact");

    a = CDR(a);

    ASSERT_TRUE(a != NULL);
}

TEST(parse_quote)
{
    int pos = 0;

    struct atom *result = parse("'foobar", &pos);
    ASSERT_TRUE(result != NULL);
    ASSERT_EQ(ATOM_LIST, result->type);

    struct atom *op = CAR(result->list);
    ASSERT_TRUE(op != NULL);
    ASSERT_EQ(ATOM_SYMBOL, op->type);
    ASSERT_STREQ("quote", op->str.str);

    struct atom *a = CDR(op);
    ASSERT_TRUE(a != NULL);
    ASSERT_EQ(ATOM_SYMBOL, a->type);
    ASSERT_STREQ("foobar", a->str.str);
}

#endif
