#include "eval.h"
#include "atom.h"
#include "parse.h"
#include "env.h"

#include <stdio.h>
#include <string.h>

static int atom_cmp(struct atom *a, struct atom *b)
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
            if (a->l != b->l)
                result = 0;
            break;

        case ATOM_STR:
        case ATOM_SYMBOL:
            if (strcmp(a->str.str, b->str.str) != 0)
                result = 0;
            break;

        case ATOM_LIST:
        {
            struct atom *ai = LIST_FIRST(a->list);
            struct atom *bi = LIST_FIRST(b->list);

            while (ai && bi)
            {
                if (!atom_cmp(ai, bi))
                {
                    result = 0;
                    break;
                }

                ai = LIST_NEXT(ai, entries);
                bi = LIST_NEXT(bi, entries);
            }

            if (ai != NULL || bi != NULL)
                result = 0;

            break;
        }
    }

    return result;
}

struct atom *eval(struct atom *expr, struct env *env)
{
    if (IS_SYM(expr))
    {
        struct atom *atom = env_lookup(env, expr->str.str);

        if (atom)
        {
            return atom;
        }
        else
        {
            printf("error: undefined variable: %s\n",
                expr->str.str);
            return &nil_atom;
        }
    }

    if (!IS_LIST(expr))
        return expr;

    struct list *list = expr->list;

    struct atom *op = LIST_FIRST(list);

    if (strcmp(op->str.str, "quote") == 0)
    {
        return atom_clone(LIST_NEXT(op, entries));
    }
    else if (strcmp(op->str.str, "atom") == 0)
    {
        struct atom *a = LIST_NEXT(op, entries);

        if (!a)
            return &nil_atom;

        if (IS_LIST(a))
            return &false_atom;
        else
            return &true_atom;
    }
    else if (strcmp(op->str.str, "eq") == 0)
    {
        struct atom *a = CDR(op);
        struct atom *b = CDR(a);

        if (!a || !b)
        {
            printf("error: eq takes 2 arguments\n");
            return &nil_atom;
        }

        a = eval(a, env);
        b = eval(b, env);

        if (atom_cmp(a, b))
            return &true_atom;

        return &false_atom;
    }
    else if (strcmp(op->str.str, "+") == 0 ||
             strcmp(op->str.str, "-") == 0 ||
             strcmp(op->str.str, "/") == 0 ||
             strcmp(op->str.str, "*") == 0)
    {
        struct atom *a = CDR(op);
        struct atom *b = CDR(a);

        if (!a || !b)
        {
            printf("error: %s takes 2 arguments\n", op->str.str);
            return &nil_atom;
        }

        a = eval(a, env);
        b = eval(b, env);

        if (!(ATOM_TYPE(a) == ATOM_TYPE(b) && ATOM_TYPE(a) == ATOM_INT))
            return &nil_atom;

        switch (*op->str.str)
        {
            case '+': return atom_new_int(a->l + b->l);
            case '-': return atom_new_int(a->l - b->l);
            case '/': return atom_new_int(a->l / b->l);
            case '*': return atom_new_int(a->l * b->l);
        }

        return &nil_atom;
    }
    else if (strcmp(op->str.str, ">") == 0)
    {
        struct atom *a = CDR(op);
        struct atom *b = CDR(a);

        if (!a || !b)
        {
            printf("error: > takes 2 arguments\n");
            return &nil_atom;
        }

        a = eval(a, env);
        b = eval(b, env);

        if (!(ATOM_TYPE(a) == ATOM_TYPE(b) && ATOM_TYPE(a) == ATOM_INT))
            return &nil_atom;

        if (a->l > b->l)
            return &true_atom;

        return &false_atom;
    }
    else if (strcmp(op->str.str, "if") == 0)
    {
        struct atom *predicate = CDR(op);
        struct atom *true_case = CDR(predicate);
        struct atom *false_case = CDR(true_case);

        if (!predicate || !true_case || !false_case)
        {
            printf("error: if takes 3 arguments\n");
            return &nil_atom;
        }

        predicate = eval(predicate, env);

        if (IS_TRUE(predicate))
            return eval(true_case, env);

        return eval(false_case, env);
    }
    else if (strcmp(op->str.str, "mod") == 0)
    {
        struct atom *a = CDR(op);
        struct atom *b = CDR(a);

        if (!a || !b)
        {
            printf("error: mod takes two arguments\n");
            return &nil_atom;
        }

        a = eval(a, env);
        b = eval(b, env);

        if (!IS_INT(a) || !IS_INT(b))
        {
            printf("error: mod arguments must be integers\n");
            return &nil_atom;
        }

        return atom_new_int(a->l % b->l);
    }
    else if (strcmp(op->str.str, "define") == 0)
    {
        struct atom *expr_name = CDR(op);
        struct atom *expr_value = CDR(expr_name);

        if (!expr_name || !expr_value)
        {
            printf("error: define takes two arguments\n");
            return &nil_atom;
        }

        if (!IS_SYM(expr_name))
        {
            printf("error: define: first arg must be symbol\n");
            return &nil_atom;
        }

        expr_value = eval(expr_value, env);

        if (!env_set(env, expr_name->str.str, expr_value))
        {
            printf("error: cannot redefine %s\n", expr_name->str.str);
            return &nil_atom;
        }

        return expr_value;
    }

    printf("error: unknown function %s\n", op->str.str);

    return &nil_atom;
}

struct atom *eval_str(const char *expr, struct env *env)
{
    struct atom *result;
    int pos = 0;

    result = eval(parse(expr, &pos), env);

    return result;
}

#ifdef BUILD_TEST

#include "test_util.h"

TEST(nested_expression)
{
    struct env *env = env_new();
    struct atom *result = eval_str("(eq #f (> (- (+ 1 3) (* 2 (mod 7 4))) 4))", env);
    ASSERT_TRUE(result != NULL);
    ASSERT_TRUE(IS_TRUE(result));
}

TEST(basic_if)
{
    struct env *env = env_new();
    struct atom *result = eval_str("(if #t 42 1000)", env);
    print_atom(result, 0);
    ASSERT_TRUE(result != NULL);
    ASSERT_EQ(ATOM_INT, ATOM_TYPE(result));
    ASSERT_EQ(42, result->l);
}

TEST(if_with_sub_expressions)
{
    struct env *env = env_new();
    struct atom *result = eval_str("(if (> 1 2) (- 1000 1) (+ 40 (- 3 1)))", env);
    ASSERT_TRUE(result != NULL);
    ASSERT_EQ(ATOM_INT, ATOM_TYPE(result));
    ASSERT_EQ(42, result->l);
}

TEST(evaluate_symbol)
{
    struct env *env = env_new();
    env_set(env, "foo", atom_new_int(42));

    struct atom *result = eval_str("foo", env);

    ASSERT_TRUE(result != NULL);
    ASSERT_EQ(ATOM_INT, ATOM_TYPE(result));
    ASSERT_EQ(42, result->l);
}

TEST(evaluate_missing_symbol)
{
    struct env *env = env_new();

    struct atom *result = eval_str("foo", env);

    ASSERT_TRUE(result != NULL);
    ASSERT_EQ(ATOM_NIL, ATOM_TYPE(result));
}

TEST(define)
{
    struct env *env = env_new();

    struct atom *result = eval_str("(define x 100)", env);

    ASSERT_TRUE(result != NULL);

    struct atom *atom = env_lookup(env, "x");

    ASSERT_TRUE(atom != NULL);
    ASSERT_EQ(ATOM_INT, atom->type);
    ASSERT_EQ(100, atom->l);
}

TEST(define_missing_value)
{
    struct env *env = env_new();

    struct atom *result = eval_str("(define x)", env);
    ASSERT_TRUE(result != NULL);
    ASSERT_EQ(ATOM_NIL, ATOM_TYPE(result));

    struct atom *atom = env_lookup(env, "x");
    ASSERT_TRUE(atom == NULL);
}

TEST(define_nonsymbol_as_name)
{
    struct env *env = env_new();

    struct atom *result = eval_str("(define 1 100)", env);
    ASSERT_TRUE(result != NULL);
    ASSERT_EQ(ATOM_NIL, ATOM_TYPE(result));
}

TEST(define_with_val_as_expr)
{
    struct env *env = env_new();

    struct atom *result = eval_str("(define x (* 3 3))", env);

    ASSERT_TRUE(result != NULL);

    struct atom *atom = env_lookup(env, "x");

    ASSERT_TRUE(atom != NULL);
    ASSERT_EQ(ATOM_INT, atom->type);
    ASSERT_EQ(9, atom->l);
}

#endif /* BUILD_TEST */
