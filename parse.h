#ifndef PARSER_H
#define PARSER_H

struct atom;
struct list;

int parse_next(const char *src, int *pos, struct atom **result);
struct list *parse(const char *src, int *pos);

void print_list(struct list *list, int level);

#endif
