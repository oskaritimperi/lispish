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

struct atom *builtin_quote(struct atom *expr, struct env *env)
{
    struct list *list = expr->list;
    struct atom *op = LIST_FIRST(list);
    (void) env;
    return atom_clone(LIST_NEXT(op, entries));
}

struct atom *builtin_atom(struct atom *expr, struct env *env)
{
    struct list *list = expr->list;
    struct atom *op = LIST_FIRST(list);
    struct atom *a = LIST_NEXT(op, entries);

    (void) env;

    if (!a)
        return &nil_atom;

    if (IS_LIST(a))
        return &false_atom;
    else
        return &true_atom;
}

struct atom *builtin_eq(struct atom *expr, struct env *env)
{
    struct list *list = expr->list;
    struct atom *op = LIST_FIRST(list);
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

struct atom *builtin_basic_arithmetic(struct atom *expr, struct env *env)
{
    struct list *list = expr->list;
    struct atom *op = LIST_FIRST(list);
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
    {
        printf("error: %s works only for integers at the moment\n",
            op->str.str);
        return &nil_atom;
    }

    switch (*op->str.str)
    {
        case '+': return atom_new_int(a->l + b->l);
        case '-': return atom_new_int(a->l - b->l);
        case '/': return atom_new_int(a->l / b->l);
        case '*': return atom_new_int(a->l * b->l);
    }

    return &nil_atom;
}

struct atom *builtin_gt(struct atom *expr, struct env *env)
{
    struct list *list = expr->list;
    struct atom *op = LIST_FIRST(list);
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

struct atom *builtin_if(struct atom *expr, struct env *env)
{
    struct list *list = expr->list;
    struct atom *op = LIST_FIRST(list);
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

struct atom *builtin_mod(struct atom *expr, struct env *env)
{
    struct list *list = expr->list;
    struct atom *op = LIST_FIRST(list);
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

struct atom *builtin_define(struct atom *expr, struct env *env)
{
    struct list *list = expr->list;
    struct atom *op = LIST_FIRST(list);
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

struct atom *builtin_lambda(struct atom *expr, struct env *env)
{
    struct list *list = expr->list;
    struct atom *op = LIST_FIRST(list);

    struct atom *params = CDR(op);
    struct atom *body = CDR(params);

    if (!params || !body || CDR(body))
    {
        printf("error: lambda takes exactly 2 arguments\n");
        return &nil_atom;
    }

    if (!IS_LIST(params) && !IS_NIL(params))
    {
        printf("error: first arg to lambda must be a list\n");
        return &nil_atom;
    }

    return atom_new_closure(params, body, env);
}

typedef struct atom *(*builtin_function_t)(struct atom *, struct env *);

static struct builtin_function_def
{
    const char *name;
    builtin_function_t fn;
} builtin_function_defs[] = {
    { "quote", &builtin_quote },
    { "atom", &builtin_atom },
    { "eq", &builtin_eq },
    { "+", &builtin_basic_arithmetic },
    { "-", &builtin_basic_arithmetic },
    { "/", &builtin_basic_arithmetic },
    { "*", &builtin_basic_arithmetic },
    { ">", &builtin_gt },
    { "if", &builtin_if },
    { "mod", &builtin_mod },
    { "define", &builtin_define },
    { "lambda", &builtin_lambda },

    { NULL, NULL }
};

struct atom *eval_closure(struct atom *closure, struct atom *args,
    struct env *env)
{
    struct env *closure_env = closure->closure.env;

    struct atom *param_value = args;
    struct atom *param_name = CAR(closure->closure.params->list);

    while (param_value && param_name)
    {
        struct atom *evaluated_param = eval(param_value, env);

        closure_env = env_extend(closure_env, 1,
            param_name->str.str, evaluated_param);

        param_value = CDR(param_value);
        param_name = CDR(param_name);
    }

    if (param_value && !param_name)
    {
        printf("error: incorrect number of arguments\n");
        return &nil_atom;
    }

    if (!param_value && param_name)
    {
        printf("error: incorrect number of arguments\n");
        return &nil_atom;
    }

    return eval(closure->closure.body, closure_env);
}

struct atom *eval(struct atom *expr, struct env *env)
{
    // symbols and not-a-lists are evaluated or returned directly

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

    // Check if the first elem is not a symbol or a closure. If it's
    // not, then we'll evaluate it (it could be a lambda form).

    if (!IS_SYM(op) && !IS_CLOSURE(op))
    {
        struct atom *evaluated_op = eval(op, env);
        // Replace the evaluated one to the list!
        LIST_REMOVE(op, entries);
        LIST_INSERT_HEAD(list, evaluated_op, entries);
        op = evaluated_op;
    }

    // If the first elem is a symbol, it should be a name for a builtin
    // function or a closure bound to that name by the user. If the
    // first argument is directly a closure, eval that with the args.

    if (IS_SYM(op))
    {
        struct builtin_function_def *def = builtin_function_defs;
        while (def->name && def->fn)
        {
            if (strcmp(op->str.str, def->name) == 0)
            {
                return def->fn(expr, env);
            }

            ++def;
        }

        struct atom *closure = env_lookup(env, op->str.str);

        if (closure)
        {
            return eval_closure(closure, CDR(op), env);
        }

        printf("error: unknown function %s\n", op->str.str);
    }
    else if (IS_CLOSURE(op))
    {
        return eval_closure(op, CDR(op), env);
    }

    printf("error: cannot evaluate\n");

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

TEST(lambda_evaluates_to_closure)
{
    struct env *env = env_new();
    struct atom *result = eval_str("(lambda () 1)", env);
    ASSERT_TRUE(result != NULL);
    ASSERT_TRUE(IS_CLOSURE(result));
}

TEST(lambda_closure_keeps_defining_env)
{
    struct env *env = env_new();

    env_set(env, "foo", atom_new_int(1));
    env_set(env, "bar", atom_new_int(2));

    struct atom *result = eval_str("(lambda () 42)", env);

    ASSERT_TRUE(result != NULL);
    ASSERT_EQ(env, result->closure.env);
}

TEST(lambda_closure_holds_function)
{
    struct env *env = env_new();

    env_set(env, "foo", atom_new_int(1));
    env_set(env, "bar", atom_new_int(2));

    struct atom *result = eval_str("(lambda (x y) (+ x y))", env);

    struct atom *params = result->closure.params;
    struct atom *body = result->closure.body;

    ASSERT_TRUE(result != NULL);
    ASSERT_TRUE(params != NULL);
    ASSERT_TRUE(body != NULL);

    ASSERT_TRUE(IS_LIST(params));

    ASSERT_TRUE(IS_SYM(CAR(params->list)));
    ASSERT_STREQ("x", CAR(params->list)->str.str);

    ASSERT_TRUE(IS_SYM(CDR(CAR(params->list))));
    ASSERT_STREQ("y", CDR(CAR(params->list))->str.str);

    ASSERT_TRUE(IS_LIST(body));

    ASSERT_TRUE(IS_SYM(CAR(body->list)));
    ASSERT_STREQ("+", CAR(body->list)->str.str);
}

TEST(lambda_args_are_lists)
{
    struct env *env = env_new();
    ASSERT_FALSE(IS_NIL(eval_str("(lambda () 1)", env)));
    ASSERT_TRUE(IS_NIL(eval_str("(lambda 1 1)", env)));
}

TEST(lambda_number_of_arguments)
{
    struct env *env = env_new();
    ASSERT_FALSE(IS_NIL(eval_str("(lambda () 1)", env)));
    ASSERT_TRUE(IS_NIL(eval_str("(lambda () () ())", env)));
}

TEST(define_lambda_with_error_in_body)
{
    struct env *env = env_new();
    struct atom *result = eval_str("(lambda (x y) (function body ((that) would never) work))", env);
    ASSERT_TRUE(result != NULL);
    ASSERT_FALSE(IS_NIL(result));
}

TEST(evaluating_call_to_closure)
{
    struct env *env = env_new();

    struct atom *closure = eval_str("(lambda () (+ 1 2))", env);

    struct atom *list = atom_new_list_empty();
    LIST_INSERT_HEAD(list->list, closure, entries);

    struct env *env2 = env_new();
    struct atom *result = eval(list, env2);

    ASSERT_TRUE(result != NULL);
    ASSERT_TRUE(IS_INT(result));
    ASSERT_EQ(3, result->l);
}

TEST(evaluating_call_to_closure_with_args)
{
    struct env *env = env_new();

    struct atom *closure = eval_str("(lambda (a b) (+ a b))", env);

    struct atom *list = atom_new_list_empty();
    LIST_INSERT_HEAD(list->list, closure, entries);
    struct atom *a = atom_new_int(4);
    struct atom *b = atom_new_int(5);
    LIST_INSERT_AFTER(closure, a, entries);
    LIST_INSERT_AFTER(a, b, entries);

    struct atom *result = eval(list, env);

    ASSERT_TRUE(result != NULL);
    ASSERT_TRUE(IS_INT(result));
    ASSERT_EQ(9, result->l);
}

TEST(call_to_function_should_eval_args)
{
    struct env *env = env_new();

    struct atom *closure = eval_str("(lambda (a) (+ a 5))", env);

    int pos = 0;

    struct atom *list = atom_list_append(atom_new_list_empty(), 2,
        closure, parse("(if #f 0 (+ 10 10))", &pos));

    struct atom *result = eval(list, env);

    ASSERT_TRUE(result != NULL);
    ASSERT_FALSE(IS_NIL(result));
    ASSERT_TRUE(IS_INT(result));
    ASSERT_EQ(25, result->l);
}

TEST(evaluating_call_to_closure_with_free_vars)
{
    struct env *env = env_new();
    env_set(env, "y", atom_new_int(1));

    struct atom *closure = eval_str("(lambda (x) (+ x y))", env);

    struct atom *l = atom_list_append(atom_new_list_empty(), 2,
        closure, atom_new_int(0));

    struct env *env2 = env_new();
    env_set(env2, "y", atom_new_int(2));

    struct atom *result = eval(l, env2);

    ASSERT_TRUE(result != NULL);
    ASSERT_TRUE(IS_INT(result));
    ASSERT_EQ(1, result->l);
}

TEST(calling_very_simple_func_in_env)
{
    struct env *env = env_new();
    eval_str("(define add (lambda (x y) (+ x y)))", env);
    ASSERT_TRUE(IS_CLOSURE(env_lookup(env, "add")));

    struct atom *result = eval_str("(add 1 2)", env);
    ASSERT_TRUE(IS_INT(result));
    ASSERT_EQ(3, result->l);
}

TEST(calling_lambda_directly)
{
    int pos = 0;
    struct atom *a = parse("((lambda (x) x) 42)", &pos);
    struct atom *result = eval(a, env_new());
    ASSERT_TRUE(result != NULL);
    ASSERT_TRUE(IS_INT(result));
    ASSERT_EQ(42, result->l);
}

TEST(calling_complex_expression_which_evaluates_to_function)
{
    struct env *env = env_new();
    env_set(env, "y", atom_new_int(3));

    struct atom *result = eval_str("((if #f wont-evaluate-me (lambda (x) (+ x y))) 2)", env);

    ASSERT_TRUE(result != NULL);
    ASSERT_TRUE(IS_INT(result));
    ASSERT_EQ(5, result->l);
}

TEST(calling_atom_fails)
{
    struct env *env = env_new();
    struct atom *result = eval_str("(#t 'foo 'bar)", env);
    ASSERT_TRUE(result != NULL);
    ASSERT_TRUE(IS_NIL(result));

    result = eval_str("(42)", env);
    ASSERT_TRUE(result != NULL);
    ASSERT_TRUE(IS_NIL(result));
}

TEST(make_sure_args_to_func_are_evaluated)
{
    struct env *env = env_new();
    struct atom *result = eval_str("((lambda (x) x) (+ 1 2))", env);
    ASSERT_TRUE(result != NULL);
    ASSERT_TRUE(IS_INT(result));
    ASSERT_EQ(3, result->l);
}

TEST(calling_with_wrong_number_of_args)
{
    struct env *env = env_new();

    eval_str("(define fn (lambda (p1 p2) 'foobar))", env);

    struct atom *result = eval_str("(fn 1 2 3)", env);

    ASSERT_TRUE(result != NULL);
    ASSERT_TRUE(IS_NIL(result));
}

TEST(calling_function_recursively)
{
    struct env *env = env_new();

    eval_str("(define fn (lambda (x) (if (eq x 0) 42 (fn (- x 1)))))", env);

    struct atom *result = eval_str("(fn 0)", env);
    ASSERT_TRUE(result != NULL);
    ASSERT_TRUE(IS_INT(result));
    ASSERT_EQ(42, result->l);

    result = eval_str("(fn 10)", env);
    ASSERT_TRUE(result != NULL);
    ASSERT_TRUE(IS_INT(result));
    ASSERT_EQ(42, result->l);
}

TEST(fibonacci)
{
    struct env *env = env_new();

    const char *src =
        "(define fibonacci "
            "(lambda (x) "
                "(if (eq x 0) "
                    "0 "
                    "(if (eq x 1) "
                        "1 "
                        "(+ (fibonacci (- x 2)) "
                           "(fibonacci (- x 1)))))))";

    struct atom *result = eval_str(src, env);

    ASSERT_TRUE(result != NULL);
    ASSERT_FALSE(IS_NIL(result));

#define ASSERT_INT_VAL(ATOM, VALUE) \
    ASSERT_TRUE((ATOM) != NULL); \
    ASSERT_TRUE(IS_INT(ATOM)); \
    ASSERT_EQ((VALUE), (ATOM)->l)

    result = eval_str("(fibonacci 0)", env);
    ASSERT_INT_VAL(result, 0);

    result = eval_str("(fibonacci 1)", env);
    ASSERT_INT_VAL(result, 1);

    result = eval_str("(fibonacci 2)", env);
    ASSERT_INT_VAL(result, 1);

    result = eval_str("(fibonacci 3)", env);
    ASSERT_INT_VAL(result, 2);

    result = eval_str("(fibonacci 4)", env);
    ASSERT_INT_VAL(result, 3);

    result = eval_str("(fibonacci 5)", env);
    ASSERT_INT_VAL(result, 5);

    result = eval_str("(fibonacci 6)", env);
    ASSERT_INT_VAL(result, 8);

    result = eval_str("(fibonacci 7)", env);
    ASSERT_INT_VAL(result, 13);
}

#endif /* BUILD_TEST */
