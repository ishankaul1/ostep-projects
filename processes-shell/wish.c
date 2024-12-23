#include <stdlib.h>
#include <stdio.h>
#include <string.h>
#include <errno.h>
#include <unistd.h>

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
    *tmp = NULL;
    return argv;
}

int parse_and_run_line(char *line)
{
    char **parsed_args = parse_args(line);

    printf("Debug - parsed arguments:\n");
    for (int i = 0; parsed_args[i] != NULL; i++)
    {
        printf("arg[%d]: '%s'\n", i, parsed_args[i]);
    }

    int rc = fork();

    if (rc < 0)
    {
        fprintf(stderr, "Fork failed\n"); // debug
        return 1;
    }
    else if (rc == 0)
    {
        // in child - run exec
        execv(parsed_args[0], parsed_args);
        fprintf(stderr, "Exec failed\n"); // debug
        // Exit, don't return. The child will continue to run the shell loop otherwise.
        exit(1);
    }
    else
    {
        // in parent - wait for child
        int wc = wait(NULL);
    }

    free(parsed_args);
    return 0;
}

int process_input_file(FILE *fp, int print_wish)
{
    char *line = NULL;
    size_t buf_size = 0;
    ssize_t line_size = 0;

    int caught_error = 0;

    while (1)
    {
        if (print_wish)
        {
            printf("wish>");
        }
        // Prevent errno from other parts of the program from propagating through; I just want to know what happened to getline
        errno = 0;
        line_size = getline(&line, &buf_size, fp);
        if (line_size == -1)
        {
            // Special handling for EOF case here - just exit
            if (errno == 0)
            {
                free(line);
                return 0;
            }
            caught_error = 1;
        }

        if (!caught_error)
        {
            if (line_size > 0)
            {
                // Getline will include newline, which throws off exec
                line[line_size - 1] = '\0';
            }
            caught_error = parse_and_run_line(line);
        }

        if (caught_error)
        {
            fprintf(stderr, "An error has occurred\n");
            caught_error = 0;
        }

        if (line != NULL)
        {
            free(line);
            line = NULL;
        }
    }

    return 0;
}


// TODO next - built-in commands, paths, & redirects

int main(int argc, char **argv)
{
    if (argc > 2)
    {
        fprintf(stderr, "wish: expected 0 or 1 arguments but got %d", argc - 1);
        exit(1);
    }
    else if (argc == 2)
    {
        FILE *in_fp = fopen(argv[1], "r");
        if (!in_fp)
        {
            fprintf(stderr, "wish: cannot open file '%s'\n", argv[1]);
            return 1;
        }
        process_input_file(in_fp, 0);
        fclose(in_fp);
    }
    else if (argc == 1)
    {
        process_input_file(stdin, 1);
    }
    fprintf(stdout, "\n");
}