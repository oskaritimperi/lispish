#ifndef PARSER_H
#define PARSER_H

struct atom;

struct atom *parse(const char *src, int *pos);

#endif
