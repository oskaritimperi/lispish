#include <stdlib.h>
#include <stdio.h>
#include <string.h>

#include "parse.h"
#include "eval.h"
#include "env.h"
#include "atom.h"
#include "linenoise.h"

int main(int argc, char **argv)
{
    char *line;
    struct env *env;

    env = env_new();

    linenoiseSetMultiLine(0);

    while ((line = linenoise("> ")) != NULL)
    {
        linenoiseHistoryAdd(line);

        if (strcmp(".clean", line) == 0)
        {
            env_free(env);
            env = env_new();
        }
        else
        {
            struct atom *result = eval_str(line, env);
            print_atom(result, 0);
        }

        free(line);
    }

    return 0;
}
