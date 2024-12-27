#include <stdlib.h>
#include <stdio.h>
#include <string.h>
#include <errno.h>
#include <unistd.h>

#define MAX_STRING_ARRAY_SIZE 100

char **parse_args(char *input_line, int *argc)
{
    // TODO for fun maybe - unbounded input size
    char **tmp, **argv = malloc(sizeof(char *) * MAX_STRING_ARRAY_SIZE);
    // From strsep man pages
    // Put chars from input_line up until whitespace into where *tmp is pointing to (dereferenced to a 'slot' in argv)
    for (tmp = argv; (*tmp = strsep(&input_line, " \t")) != NULL;)
    {
        if (**tmp != '\0')
        {
            (*argc)++;
            if (++tmp >= &argv[100])
            {
                break;
            }
        }
    }
    *tmp = NULL;
    return argv;
}

// TODO - path resolution
int _handle_arbitrary_command(char *parsed_args)
{
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
    return 0;
}

int _handle_path(char **path_args, char **path, int *path_size)
{
    // skip 'path'
    ++path_args;
    while (*path_args != NULL)
    {
        if (path_size >= MAX_STRING_ARRAY_SIZE)
        {
            return 1;
        }
        char *tmp = strdup(*path_args);
        if (tmp == NULL)
        {
            return 1;
        }
        path[*(path_size)++] = tmp;
        path_args++;
    }

    return 0;
}

int _handle_cd(char **cd_args, int cd_argc)
{
    if (cd_argc != 2)
    {
        return 1;
    }
    if (chdir(cd_args[1]) != 0)
    {
        return 1;
    }
    return 0;
}

// NOTE: When implementing operators, split parsing of line from running of a single command.
int parse_and_run_line(char *line, char **path, int *path_size, int *should_exit)
{
    int argc = 0;
    char **parsed_args = parse_args(line, &argc);
    char *cmd = parsed_args[0];

    // printf("Debug - parsed arguments:\n");
    // for (int i = 0; parsed_args[i] != NULL; i++)
    // {
    //     printf("arg[%d]: '%s'\n", i, parsed_args[i]);
    // }

    /*
        Built-in commands:
            - path: take in one or more arguments, concat them to the path
            - exit: just make a call to exit(0). error if any args
            - cd: take in one and only one arg, call chdir
    */

    int ret = 0;

    if (strcmp(cmd, "path") == 0)
    {
        ret = _handle_path(parsed_args, path, path_size);
    }
    else if (strcmp(cmd, "exit") == 0)
    {
        if (argc > 1)
        {
            ret = 1;
        }
        else
        {
            *should_exit = 1;
        }
    }
    else if (strcmp(cmd, "cd") == 0)
    {
        ret = _handle_cd(parsed_args, argc);
    }
    else
    {
        ret = _handle_arbitrary_command(parsed_args);
    }

    free(parsed_args);
    return ret;
}

int _inject_bin_into_path(char **path, int *path_size)
{
    char *bin = strdup("/bin");
    if (bin == NULL)
    {
        return 1;
    }
    path[*(path_size)++] = bin;
    return 0;
}

int _free_arr_of_strings(char **arr, int arr_size)
{
    for (int i = 0; i < arr_size; i++)
    {
        free(arr[i]);
    }
    free(arr);
    return 0;
}

int process_input_file(FILE *fp, int print_wish)
{
    char *line = NULL;
    size_t buf_size = 0;
    ssize_t line_size = 0;

    char **path = malloc(sizeof(char *) * MAX_STRING_ARRAY_SIZE);
    int path_size = 0;

    if (path == NULL)
    {
        return 1;
    }
    if (_inject_bin_into_path(path, &path_size) != 0)
    {
        _free_arr_of_strings(path, path_size);
        return 1;
    }

    int caught_error = 0;
    int should_exit = 0;

    while (!should_exit)
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
                _free_arr_of_strings(path, path_size);
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
            caught_error = parse_and_run_line(line, path, &path_size, &should_exit);
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

    _free_arr_of_strings(path, path_size);
    return 0;
}

// TODO next - path resolution & redirects

int main(int argc, char **argv)
{
    if (argc > 2)
    {
        fprintf(stderr, "wish: expected 0 or 1 arguments but got %d", argc - 1);
        exit(1);
    }
    int ret = 0;
    if (argc == 2)
    {
        FILE *in_fp = fopen(argv[1], "r");
        if (!in_fp)
        {
            fprintf(stderr, "wish: cannot open file '%s'\n", argv[1]);
            return 1;
        }
        ret = process_input_file(in_fp, 0);
        fclose(in_fp);
    }
    else if (argc == 1)
    {
        ret = process_input_file(stdin, 1);
    }
    fprintf(stdout, "\n");
    return ret;
}