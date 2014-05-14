#include "eval.h"
#include "list.h"
#include "atom.h"
#include "parse.h"

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
    if (!IS_LIST(list))
        return list;

    struct list *l = LIST_GET_ATOM(list)->list;

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
    }
    else if (IS_LIST(l))
    {
        return eval(l);
    }

    return list_append(NULL, &nil_atom);
}

struct list *eval_str(const char *str)
{
    struct list *result;
    int pos = 0;

    result = eval(parse(str, &pos));

    return result;
}

#ifdef BUILD_TEST

#include "test_util.h"

TEST(eval)
{
    struct list *result;
    int pos;

#define EVAL(EXPR) \
    pos = 0; \
    result = eval(parse((EXPR), &pos))

#define ARITHMETIC_TEST(EXPR, RESULT) \
    EVAL(EXPR); \
    ASSERT_EQ(ATOM_INT, ATOM_TYPE(result)); \
    ASSERT_EQ(RESULT, LIST_GET_ATOM(result)->l)

    ARITHMETIC_TEST("(+ 1 2)", 3);
    ARITHMETIC_TEST("(- 5 10)", -5);
    ARITHMETIC_TEST("(/ 42 2)", 21);
    ARITHMETIC_TEST("(* 5 10)", 50);
    ARITHMETIC_TEST("(* (* 2 (+ 1 1)) 2)", 8);

#undef ARITHMETIC_TEST

#define EQ_TEST(EXPR, RESULT) \
    EVAL(EXPR); \
    ASSERT_EQ(result, RESULT)

#define EQ_TEST_T(EXPR) EVAL(EXPR); ASSERT(IS_TRUE(result))
#define EQ_TEST_F(EXPR) EVAL(EXPR); ASSERT(IS_FALSE(result))

    EQ_TEST_T("(eq 1 1)");
    EQ_TEST_T("(eq (+ 1 1) 2)");
    EQ_TEST_T("(eq (quote (1 2 3)) (quote (1 2 3)))");

    EQ_TEST_T("(eq \"eka\" \"eka\"");
    EQ_TEST_F("(eq \"eka\" eka)");
    EQ_TEST_F("(eq \"eka\" 100)");
    EQ_TEST_F("(eq \"eka\" \"toka\"");

    EQ_TEST_T("(eq eka eka)");
    EQ_TEST_F("(eq eka toka)");

    EQ_TEST_F("(eq 1 2)");
    EQ_TEST_F("(eq 1 (- 1 1))");
    EQ_TEST_F("(eq (quote (1)) (quote (1 2 3)))");

    EQ_TEST_T("(eq (quote (1 2)) '(1 2))");
    EQ_TEST_T("(eq 'bar 'bar)");
    EQ_TEST_F("(eq 'foo 'bar)");
    EQ_TEST_T("(eq (quote bar) 'bar)");
    EQ_TEST_F("(eq (quote foo) 'bar)");

    EQ_TEST_F("(> 1 2)");
    EQ_TEST_T("(> 2 1)");

#undef EQ_TEST_F
#undef EQ_TEST_T
#undef EQ_TEST

    EVAL("(if #t 1 2)");
    ASSERT_EQ(1, LIST_GET_ATOM(result)->l);

    EVAL("(if #t (+ 1 1) (* 2 2))");
    ASSERT_EQ(2, LIST_GET_ATOM(result)->l);

    EVAL("(if #f (+ 1 1) (* 2 2))");
    ASSERT_EQ(4, LIST_GET_ATOM(result)->l);

#undef EVAL
}

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

#endif /* BUILD_TEST */
