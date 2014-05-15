#include "env.h"
#include "list.h"
#include "atom.h"

#include <stdlib.h>
#include <stdarg.h>
#include <string.h>

struct kv
{
    const char *symbol;
    struct atom *value;
};

struct list *env_new()
{
    return list_new(NULL);
}

struct atom *env_lookup(struct list *env, const char *symbol)
{
    env = env->next;
    while (env)
    {
        struct kv *kv = (struct kv *) env->data;

        if (strcmp(kv->symbol, symbol) == 0)
        {
            return kv->value;
        }

        env = env->next;
    }
    return NULL;
}

int env_set_(struct list *env, const char *symbol,
    struct atom *atom, int force)
{
    struct list *first = env;
    struct kv *kv;

    env = env->next;
    while (env)
    {
        kv = (struct kv *) env->data;

        if (strcmp(symbol, kv->symbol) == 0)
        {
            if (!force)
                return 0;

            kv->value = atom;

            return 1;
        }

        if (!env->next)
            break;

        env = env->next;
    }

    if (!env)
        env = first;

    kv = calloc(1, sizeof(*kv));
    kv->symbol = strdup(symbol);
    kv->value = atom;

    list_append(env, kv);

    return 1;
}

struct list *env_extend(struct list *env, int count, ...)
{
    va_list ap;
    int i;

    struct list *result = env_clone(env);

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

// struct list *env_extend_env(struct list *enva, struct list *envb)
// {

// }

int env_set(struct list *env, const char *symbol,
    struct atom *value)
{
    return env_set_(env, symbol, value, 0);
}

void env_free(struct list *env)
{

}

struct list *env_clone(struct list *env)
{
    struct list *clone = env_new();
    struct list *last = clone;

    env = env->next;
    while (env)
    {
        struct kv *kv = (struct kv *) env->data;

        struct kv *kv_clone = malloc(sizeof(*kv_clone));

        kv_clone->symbol = strdup(kv->symbol);
        kv_clone->value = atom_clone(kv->value);

        last = list_append(last, kv_clone);

        env = env->next;
    }

    return clone;
}

#ifdef BUILD_TEST

#include "test_util.h"

TEST(simple_lookup_on_empty_env)
{
    struct list *env = env_new();
    ASSERT_TRUE(env != NULL);
    ASSERT_EQ(NULL, env_lookup(env, "foobar"));
}

TEST(simple_set_and_lookup)
{
    struct list *env = env_new();
    ASSERT_TRUE(env != NULL);

    struct atom atom;

    ASSERT_EQ(1, env_set(env, "foobar", &atom));
    ASSERT_EQ(&atom, env_lookup(env, "foobar"));
}

TEST(lookup_from_inner_env)
{
    struct list *outer = env_new();

    struct atom *atom1 = atom_new_int(42);
    ASSERT_EQ(1, env_set(outer, "foo", atom1));

    struct atom *atom2 = atom_new_int(6);
    struct list *inner = env_extend(outer, 1, "bar", atom2);

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
    struct list *env = env_new();
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
    struct list *env = env_new();

    env_set(env, "foo", atom_new_int(1));

    env = env_extend(env, 1, "foo", atom_new_int(2));

    struct atom *atom = env_lookup(env, "foo");
    ASSERT_TRUE(atom != NULL);
    ASSERT_EQ(ATOM_INT, atom->type);
    ASSERT_EQ(2, atom->l);
}

TEST(redefine_illegal)
{
    struct list *env = env_new();
    ASSERT_EQ(1, env_set(env, "foo", atom_new_int(1)));
    ASSERT_EQ(0, env_set(env, "foo", atom_new_int(2)));
}

#endif /* BUILD_TEST */
