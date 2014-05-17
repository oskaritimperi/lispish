#ifndef ENV_H
#define ENV_H

#include <sys/queue.h>

struct kv;
LIST_HEAD(env, kv);

struct atom;

struct env *env_new();
struct atom *env_lookup(struct env *env, const char *symbol);
struct env *env_extend(struct env *env, int count, ...);
int env_set(struct env *env, const char *symbol,
    struct atom *value);
void env_free(struct env *env);
struct env *env_clone(struct env *env);

#endif
