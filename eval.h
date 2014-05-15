#ifndef EVAL_H
#define EVAL_H

struct list;

struct list *eval(struct list *expr);
struct list *eval_str(const char *expr);

struct list *eval_env(struct list *expr, struct list *env);
struct list *eval_str_env(const char *expr, struct list *env);

#endif
