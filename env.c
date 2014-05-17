#include "env.h"
#include "atom.h"

#include <stdlib.h>
#include <stdarg.h>
#include <string.h>

struct kv
{
    const char *symbol;
    struct atom *value;
    LIST_ENTRY(kv) entries;
};

struct env *env_new()
{
    struct env *env = calloc(1, sizeof(*env));
    LIST_INIT(env);
    return env;
}

struct atom *env_lookup(struct env *env, const char *symbol)
{
    struct kv *elem;
    LIST_FOREACH(elem, env, entries)
    {
        if (strcmp(elem->symbol, symbol) == 0)
            return elem->value;
    }

    return NULL;
}

int env_set_(struct env *env, const char *symbol,
    struct atom *atom, int force)
{
    struct kv *elem = NULL;
    struct kv *kv;

    LIST_FOREACH(elem, env, entries)
    {
        if (strcmp(symbol, elem->symbol) == 0)
        {
            if (!force)
                return 0;

            elem->value = atom;

            return 1;
        }

        if (!LIST_NEXT(elem, entries))
            break;
    }

    kv = calloc(1, sizeof(*kv));
    kv->symbol = strdup(symbol);
    kv->value = atom;

    if (LIST_EMPTY(env))
    {
        LIST_INSERT_HEAD(env, kv, entries);
    }
    else
    {
        LIST_INSERT_AFTER(elem, kv, entries);
    }

    return 1;
}

struct env *env_extend(struct env *env, int count, ...)
{
    va_list ap;
    int i;

    struct env *result = env_clone(env);

    va_start(ap, count);

    for (i = 0; i < count; ++i)
    {
        const char *symbol = va_arg(ap, const char *);
        struct atom *atom = va_arg(ap, struct atom *);
        env_set_(result, symbol, atom, 1);
    }

    va_end(ap);

    return result;
}

int env_set(struct env *env, const char *symbol,
    struct atom *value)
{
    return env_set_(env, symbol, value, 0);
}

void env_free(struct env *env)
{

}

struct env *env_clone(struct env *env)
{
    struct env *clone = env_new();
    struct kv *elem, *last;

    LIST_FOREACH(elem, env, entries)
    {
        struct kv *kv_clone = calloc(1, sizeof(*kv_clone));

        kv_clone->symbol = strdup(elem->symbol);
        kv_clone->value = atom_clone(elem->value);

        if (LIST_EMPTY(clone))
            LIST_INSERT_HEAD(clone, kv_clone, entries);
        else
            LIST_INSERT_AFTER(last, kv_clone, entries);

        last = kv_clone;
    }

    return clone;
}

#ifdef BUILD_TEST

#include "test_util.h"

TEST(simple_lookup_on_empty_env)
{
    struct env *env = env_new();
    ASSERT_TRUE(env != NULL);
    ASSERT_EQ(NULL, env_lookup(env, "foobar"));
}

TEST(simple_set_and_lookup)
{
    struct env *env = env_new();
    ASSERT_TRUE(env != NULL);

    struct atom atom;

    ASSERT_EQ(1, env_set(env, "foobar", &atom));
    ASSERT_EQ(&atom, env_lookup(env, "foobar"));
}

TEST(lookup_from_inner_env)
{
    struct env *outer = env_new();

    struct atom *atom1 = atom_new_int(42);
    ASSERT_EQ(1, env_set(outer, "foo", atom1));

    struct atom *atom2 = atom_new_int(6);
    struct env *inner = env_extend(outer, 1, "bar", atom2);

    ASSERT_TRUE(inner != NULL);
    ASSERT_TRUE(inner != outer);

    struct atom *atom = env_lookup(inner, "foo");
    ASSERT_TRUE(atom != NULL);
    ASSERT_EQ(atom1->type, atom->type);
    ASSERT_EQ(atom1->l, atom->l);

    atom = env_lookup(inner, "bar");
    ASSERT_TRUE(atom != NULL);
    ASSERT_EQ(atom2->type, atom->type);
    ASSERT_EQ(atom2->l, atom->l);
}

TEST(lookup_deeply_nested)
{
    struct env *env = env_new();
    env_set(env, "a", atom_new_int(1));
    env = env_extend(env, 1, "b", atom_new_int(2));
    env = env_extend(env, 1, "c", atom_new_int(3));
    env = env_extend(env, 1, "d", atom_new_int(4));
    env = env_extend(env, 1, "e", atom_new_int(5));

    struct atom *atom = env_lookup(env, "a");
    ASSERT_TRUE(atom != NULL);
    ASSERT_EQ(ATOM_INT, atom->type);
    ASSERT_EQ(1, atom->l);

    atom = env_lookup(env, "e");
    ASSERT_TRUE(atom != NULL);
    ASSERT_EQ(ATOM_INT, atom->type);
    ASSERT_EQ(5, atom->l);
}

TEST(extend)
{
    struct env *env = env_new();

    env_set(env, "foo", atom_new_int(1));

    env = env_extend(env, 1, "foo", atom_new_int(2));

    struct atom *atom = env_lookup(env, "foo");
    ASSERT_TRUE(atom != NULL);
    ASSERT_EQ(ATOM_INT, atom->type);
    ASSERT_EQ(2, atom->l);
}

TEST(redefine_illegal)
{
    struct env *env = env_new();
    ASSERT_EQ(1, env_set(env, "foo", atom_new_int(1)));
    ASSERT_EQ(0, env_set(env, "foo", atom_new_int(2)));
}

#endif /* BUILD_TEST */
