#include "tokens.h"

#include <ctype.h>

int get_next_token(const char *src, int *pos, struct token *token)
{
    char c;

start:

    while (isspace(src[*pos]))
    {
        *pos += 1;
    }

    c = src[*pos];

    if (!c)
        return 0;

    if (c == ';')
    {
        *pos += 1;
        while (src[*pos] != '\n')
        {
            *pos += 1;
        }

        goto start;
    }

    if (c == '(')
    {
        token->type = TOKEN_LPAREN;
        token->s = &src[*pos];
        token->len = 1;
        *pos += 1;
    }
    else if (c == ')')
    {
        token->type = TOKEN_RPAREN;
        token->s = &src[*pos];
        token->len = 1;
        *pos += 1;
    }
    else if (c == '.')
    {
        token->type = TOKEN_PERIOD;
        token->s = &src[*pos];
        token->len = 1;
        *pos += 1;
    }
    else if (c == '\'')
    {
        token->type = TOKEN_QUOTE;
        token->s = &src[*pos];
        token->len = 1;
        *pos += 1;
    }
    else if (isdigit(c))
    {
        token->type = TOKEN_INT;
        token->s = &src[*pos];

        while (!isspace(src[*pos]))
        {
            if (isdigit(src[*pos]))
            {
                *pos += 1;
            }
            else
            {
                c = src[*pos];

                if (c == '(' || c == ')')
                {
                    break;
                }

                return -1;
            }
        }

        token->len = &src[*pos] - token->s;
    }
    else if (c == '"')
    {
        *pos += 1;
        token->type = TOKEN_STR;
        token->s = &src[*pos];

        while (src[*pos] != '"')
        {
            *pos += 1;
        }

        token->len = &src[*pos] - token->s;

        *pos += 1;
    }
    else if (isalnum(c) || ispunct(c))
    {
        token->type = TOKEN_SYMBOL;
        token->s = &src[*pos];
        *pos += 1;

        while ((isalnum(src[*pos]) || ispunct(src[*pos])) && src[*pos] != '(' && src[*pos] != ')')
        {
            *pos += 1;
        }

        token->len = &src[*pos] - token->s;
    }
    else
    {
        return -1;
    }

    return 1;
}

#ifdef BUILD_TEST

#include "test_util.h"

static const char *test_src_fact =
    "(define fact\n"
    "    ;; Factorial function\n"
    "    (lambda (n)\n"
    "        (if (eq n 0)\n"
    "            1 ; Factorial of 0 is 1\n"
    "            (* n (fact (- n 1))))))\n"
    "(fact 5) ;; this should evaluate to 120\n";

TEST(test_get_next_token)
{
    int pos = 0;

    struct token token;

    ASSERT_EQ(1, get_next_token(test_src_fact, &pos, &token));
    ASSERT_EQ(TOKEN_LPAREN, token.type);

    ASSERT_EQ(1, get_next_token(test_src_fact, &pos, &token));
    ASSERT_EQ(TOKEN_SYMBOL, token.type);
    ASSERT_STREQ_N("define", token.s, 6);

    ASSERT_EQ(1, get_next_token(test_src_fact, &pos, &token));
    ASSERT_EQ(TOKEN_SYMBOL, token.type);
    ASSERT_STREQ_N("fact", token.s, 4);

    ASSERT_EQ(1, get_next_token(test_src_fact, &pos, &token));
    ASSERT_EQ(TOKEN_LPAREN, token.type);

    ASSERT_EQ(1, get_next_token(test_src_fact, &pos, &token));
    ASSERT_EQ(TOKEN_SYMBOL, token.type);
    ASSERT_STREQ_N("lambda", token.s, 6);

    ASSERT_EQ(1, get_next_token(test_src_fact, &pos, &token));
    ASSERT_EQ(TOKEN_LPAREN, token.type);

    ASSERT_EQ(1, get_next_token(test_src_fact, &pos, &token));
    ASSERT_EQ(TOKEN_SYMBOL, token.type);
    ASSERT_STREQ_N("n", token.s, 1);

    ASSERT_EQ(1, get_next_token(test_src_fact, &pos, &token));
    ASSERT_EQ(TOKEN_RPAREN, token.type);

    ASSERT_EQ(1, get_next_token(test_src_fact, &pos, &token));
    ASSERT_EQ(1, get_next_token(test_src_fact, &pos, &token));
    ASSERT_EQ(1, get_next_token(test_src_fact, &pos, &token));
    ASSERT_EQ(1, get_next_token(test_src_fact, &pos, &token));
    ASSERT_EQ(1, get_next_token(test_src_fact, &pos, &token));

    ASSERT_EQ(1, get_next_token(test_src_fact, &pos, &token));
    ASSERT_EQ(TOKEN_INT, token.type);
    ASSERT_STREQ_N("0", token.s, 1);

    ASSERT_EQ(1, get_next_token(test_src_fact, &pos, &token));
    ASSERT_EQ(1, get_next_token(test_src_fact, &pos, &token));
    ASSERT_EQ(1, get_next_token(test_src_fact, &pos, &token));

    ASSERT_EQ(1, get_next_token(test_src_fact, &pos, &token));
    ASSERT_EQ(TOKEN_SYMBOL, token.type);
    ASSERT_STREQ_N("*", token.s, 1);
}

#endif /* BUILD_TEST */
