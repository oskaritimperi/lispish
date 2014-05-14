#ifndef TOKENS_H
#define TOKENS_H

enum
{
    TOKEN_INT,
    TOKEN_STR,
    TOKEN_SYMBOL,
    TOKEN_LPAREN,
    TOKEN_RPAREN,
    TOKEN_PERIOD,
    TOKEN_QUOTE
};

struct token
{
    const char *s;
    int len;
    int type;
};

int get_next_token(const char *src, int *pos, struct token *token);

#endif
