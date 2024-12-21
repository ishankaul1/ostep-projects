#include <stdlib.h>
#include <stdio.h>
#include <string.h>
#include <errno.h>

char **parse_args(char *input_line)
{
    // TODO for fun maybe - unbounded input size
    char **tmp, **argv = malloc(sizeof(char *) * 100);
    // From strsep man pages
    // Put chars from input_line up until whitespace into where *tmp is pointing to (dereferenced to a 'slot' in argv)
    for (tmp = argv; (*tmp = strsep(&input_line, " \t")) != NULL;)
    {
        if (**tmp != '\0')
        {
            if (++tmp >= &argv[100])
            {
                break;
            }
        }
    }
    return argv;
}

int main(int argc, char **argv)
{
    if (argc == 1)
    {
        // Interactive mode
        char *line = NULL;
        size_t buf_size = 0;
        while (1)
        {
            printf("wish>");
            ssize_t chars_read = getline(&line, &buf_size, stdin);
            if (chars_read == -1)
            {
                if (errno == 0)
                {
                    return 0;
                }
                fprintf(stderr, "wish: Could not read input\n");
                return 1;
            }
            char **parsed_args = parse_args(line);

            // do fork/exec in here

            free(parsed_args);
            free(line);
            line = NULL;
        }
    }
    else if (argc == 2)
    {
        // Input file mode
    }
}