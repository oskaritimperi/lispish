#include <stdlib.h>
#include <stdio.h>

#include "parse.h"
#include "eval.h"
#include "list.h"
#include "linenoise.h"

int main(int argc, char **argv)
{
    char *line;

    linenoiseSetMultiLine(0);

    while ((line = linenoise("> ")) != NULL)
    {
        linenoiseHistoryAdd(line);

        struct list *result = eval_str(line);

        print_list(result, 0);

        free(line);
    }

    return 0;
}
