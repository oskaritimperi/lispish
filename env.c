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
