#include "eval.h"
#include "list.h"
#include "atom.h"
#include "parse.h"
#include "env.h"

#include <stdio.h>
#include <string.h>

static int atom_cmp(struct list *a, struct list *b)
{
    if (ATOM_TYPE(a) != ATOM_TYPE(b))
        return 0;

    if (IS_TRUE(a) && !IS_TRUE(b))
        return 0;

    if (IS_FALSE(a) && !IS_FALSE(b))
        return 0;

    if (IS_TRUE(b) && !IS_TRUE(a))
        return 0;

    if (IS_FALSE(b) && !IS_FALSE(a))
        return 0;

    if (IS_NIL(a) && !IS_NIL(b))
        return 0;

    if (IS_NIL(b) && !IS_NIL(a))
        return 0;

    int result = 1;

    switch (ATOM_TYPE(a))
    {
        case ATOM_INT:
            if (LIST_GET_ATOM(a)->l != LIST_GET_ATOM(b)->l)
                result = 0;
            break;

        case ATOM_STR:
        case ATOM_SYMBOL:
            if (strcmp(LIST_GET_ATOM(a)->str.str, LIST_GET_ATOM(b)->str.str) != 0)
                result = 0;
            break;

        case ATOM_LIST:
        {
            struct list *ai = LIST_GET_ATOM(a)->list;
            struct list *bi = LIST_GET_ATOM(b)->list;

            while (ai && bi)
            {
                if (!atom_cmp(ai, bi))
                {
                    result = 0;
                    break;
                }

                ai = ai->next;
                bi = bi->next;
            }

            if (ai != NULL || bi != NULL)
                result = 0;

            break;
        }
    }

    return result;
}

struct list *eval(struct list *list)
{
    struct list env;
    memset(&env, 0, sizeof(env));
    return eval_env(list, &env);
}

struct list *eval_str(const char *str)
{
    struct list env;
    memset(&env, 0, sizeof(env));
    return eval_str_env(str, &env);
}

struct list *eval_env(struct list *expr, struct list *env)
{
    if (IS_SYM(expr))
    {
        struct atom *atom = env_lookup(env, LIST_GET_ATOM(expr)->str.str);

        if (atom)
        {
            return list_append(NULL, atom);
        }
        else
        {
            printf("error: undefined variable: %s\n",
                LIST_GET_ATOM(expr)->str.str);
            return list_append(NULL, &nil_atom);
        }
    }

    if (!IS_LIST(expr))
        return expr;

    struct list *l = LIST_GET_ATOM(expr)->list;

    if (IS_SYM(l))
    {
        const char *sym = LIST_GET_ATOM(l)->str.str;

        if (strcmp(sym, "quote") == 0)
        {
            return l->next;
        }
        else if (strcmp(sym, "atom") == 0)
        {
            if (IS_LIST(l->next))
            {
                return list_append(NULL, &false_atom);
            }
            else
            {
                return list_append(NULL, &true_atom);
            }
        }
        else if (strcmp(sym, "eq") == 0)
        {
            struct list *a = CDR(l);
            struct list *b = CDR(a);

            if (!a || !b)
            {
                printf("error: eq takes 2 arguments\n");
                return list_append(NULL, &nil_atom);
            }

            a = eval(a);
            b = eval(b);

            if (atom_cmp(a, b))
                return list_append(NULL, &true_atom);

            return list_append(NULL, &false_atom);
        }
        else if (strncmp(sym, "+", 1) == 0 ||
                 strncmp(sym, "-", 1) == 0 ||
                 strncmp(sym, "/", 1) == 0 ||
                 strncmp(sym, "*", 1) == 0)
        {
            struct list *oper = CAR(l);
            struct list *a = CDR(oper);
            struct list *b = CDDR(oper);

            if (!a || !b)
                return list_append(NULL, &nil_atom);

            a = eval(a);
            b = eval(b);

            if (!(ATOM_TYPE(a) == ATOM_TYPE(b) && ATOM_TYPE(a) == ATOM_INT))
                return list_append(NULL, &nil_atom);

            long numa = LIST_GET_ATOM(a)->l;
            long numb = LIST_GET_ATOM(b)->l;
            long numr;

            switch (*sym)
            {
                case '+':
                    numr = numa + numb;
                    break;
                case '-':
                    numr = numa - numb;
                    break;
                case '/':
                    numr = numa / numb;
                    break;
                case '*':
                    numr = numa * numb;
                    break;
            }

            struct list *result = list_append(NULL, atom_new_int(numr));

            return result;
        }
        else if (strncmp(sym, ">", 1) == 0)
        {
            struct list *oper = CAR(l);
            struct list *a = CDR(oper);
            struct list *b = CDDR(oper);

            if (!a || !b)
                return list_append(NULL, &nil_atom);

            a = eval(a);
            b = eval(b);

            if (!(ATOM_TYPE(a) == ATOM_TYPE(b) && ATOM_TYPE(a) == ATOM_INT))
                return list_append(NULL, &nil_atom);

            long numa = LIST_GET_ATOM(a)->l;
            long numb = LIST_GET_ATOM(b)->l;

            if (numa > numb)
                return list_append(NULL, &true_atom);
            else
                return list_append(NULL, &false_atom);
        }
        else if (strcmp(sym, "if") == 0)
        {
            struct list *predicate = CDR(l);
            struct list *true_case = CDR(predicate);
            struct list *false_case = CDR(true_case);

            if (!predicate || !true_case || !false_case)
                return list_append(NULL, &nil_atom);

            predicate = eval(predicate);

            if (IS_TRUE(predicate))
                return eval(true_case);
            else
                return eval(false_case);
        }
        else if (strcmp(sym, "mod") == 0)
        {
            struct list *a = CDR(l);
            struct list *b = CDR(a);

            if (!a || !b)
            {
                printf("error: mod takes two arguments\n");
                return list_append(NULL, &nil_atom);
            }

            if (!IS_INT(a) || !IS_INT(b))
            {
                printf("error: mod arguments must be integers\n");
                return list_append(NULL, &nil_atom);
            }

            long result = LIST_GET_ATOM(a)->l % LIST_GET_ATOM(b)->l;

            return list_append(NULL, atom_new_int(result));
        }
        else if (strcmp(sym, "define") == 0)
        {
            struct list *expr_name = CDR(l);
            struct list *expr_value = CDR(expr_name);

            if (!expr_name || !expr_value)
            {
                printf("error: define takes two arguments\n");
                return list_append(NULL, &nil_atom);
            }

            if (!IS_SYM(expr_name))
            {
                printf("error: define: first arg must be symbol\n");
                return list_append(NULL, &nil_atom);
            }

            expr_value = eval_env(expr_value, env);

            env_set(env, LIST_GET_ATOM(expr_name)->str.str,
                LIST_GET_ATOM(expr_value));

            return list_append(NULL, expr_value);
        }
    }
    else if (IS_LIST(l))
    {
        return eval(l);
    }

    return list_append(NULL, &nil_atom);
}

struct list *eval_str_env(const char *expr, struct list *env)
{
    struct list *result;
    int pos = 0;

    result = eval_env(parse(expr, &pos), env);

    return result;
}

#ifdef BUILD_TEST

#include "test_util.h"

TEST(nested_expression)
{
    struct list *result = eval_str("(eq #f (> (- (+ 1 3) (* 2 (mod 7 4))) 4))");
    ASSERT_TRUE(result != NULL);
    ASSERT_TRUE(IS_TRUE(result));
}

TEST(basic_if)
{
    struct list *result = eval_str("(if #t 42 1000)");
    ASSERT_TRUE(result != NULL);
    ASSERT_EQ(ATOM_INT, ATOM_TYPE(result));
    ASSERT_EQ(42, LIST_GET_ATOM(result)->l);
}

TEST(if_with_sub_expressions)
{
    struct list *result = eval_str("(if (> 1 2) (- 1000 1) (+ 40 (- 3 1)))");
    ASSERT_TRUE(result != NULL);
    ASSERT_EQ(ATOM_INT, ATOM_TYPE(result));
    ASSERT_EQ(42, LIST_GET_ATOM(result)->l);
}

/* EVALUTE WITH ENVIRONMENT TESTS */

TEST(evaluate_symbol)
{
    struct list *env = env_new();
    env_set(env, "foo", atom_new_int(42));

    struct list *result = eval_str_env("foo", env);

    ASSERT_TRUE(result != NULL);
    ASSERT_EQ(ATOM_INT, ATOM_TYPE(result));
    ASSERT_EQ(42, LIST_GET_ATOM(result)->l);
}

TEST(evaluate_missing_symbol)
{
    struct list *env = env_new();

    struct list *result = eval_str_env("foo", env);

    ASSERT_TRUE(result != NULL);
    ASSERT_EQ(ATOM_NIL, ATOM_TYPE(result));
}

TEST(define)
{
    struct list *env = env_new();

    struct list *result = eval_str_env("(define x 100)", env);

    ASSERT_TRUE(result != NULL);

    struct atom *atom = env_lookup(env, "x");

    ASSERT_TRUE(atom != NULL);
    ASSERT_EQ(ATOM_INT, atom->type);
    ASSERT_EQ(100, atom->l);
}

TEST(define_missing_value)
{
    struct list *env = env_new();

    struct list *result = eval_str_env("(define x)", env);
    ASSERT_TRUE(result != NULL);
    ASSERT_EQ(ATOM_NIL, ATOM_TYPE(result));

    struct atom *atom = env_lookup(env, "x");
    ASSERT_TRUE(atom == NULL);
}

TEST(define_nonsymbol_as_name)
{
    struct list *env = env_new();

    struct list *result = eval_str_env("(define 1 100)", env);
    ASSERT_TRUE(result != NULL);
    ASSERT_EQ(ATOM_NIL, ATOM_TYPE(result));
}

TEST(define_with_val_as_expr)
{
    struct list *env = env_new();

    struct list *result = eval_str_env("(define x (* 3 3))", env);

    ASSERT_TRUE(result != NULL);

    struct atom *atom = env_lookup(env, "x");

    ASSERT_TRUE(atom != NULL);
    ASSERT_EQ(ATOM_INT, atom->type);
    ASSERT_EQ(9, atom->l);
}


#endif /* BUILD_TEST */
