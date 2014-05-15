#include <stdlib.h>
#include <stdio.h>
#include <string.h>

#include "parse.h"
#include "eval.h"
#include "list.h"
#include "env.h"
#include "linenoise.h"

int main(int argc, char **argv)
{
    char *line;
    struct list *env;

    env = env_new();

    linenoiseSetMultiLine(0);

    while ((line = linenoise("> ")) != NULL)
    {
        linenoiseHistoryAdd(line);

        if (strcmp(".clean", line) == 0)
        {
            env = env_new();
        }
        else
        {
            struct list *result = eval_str_env(line, env);
            print_list(result, 0);
        }

        free(line);
    }

    return 0;
}
