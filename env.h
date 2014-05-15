#ifndef ENV_H
#define ENV_H

struct list;
struct atom;

struct list *env_new();
struct atom *env_lookup(struct list *env, const char *symbol);
struct list *env_extend(struct list *env, int count, ...); //const char *symbol, struct atom *value
// struct list *env_extend_env(struct list *enva, struct list *envb);
int env_set(struct list *env, const char *symbol,
    struct atom *value);
void env_free(struct list *env);
struct list *env_clone(struct list *env);

#endif
