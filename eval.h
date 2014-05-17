#ifndef EVAL_H
#define EVAL_H

struct atom;
struct env;

struct atom *eval(struct atom *expr, struct env *env);
struct atom *eval_str(const char *expr, struct env *env);

#endif
