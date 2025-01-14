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

int _resolve_command_path(char* cmd, char**path, int path_size, char** resolved_command) {
    // first check if absolute
    // printf("Resolving command: %s\n", cmd);
    if (cmd[0] == '/'){
        // edge case - what if the cmd passed is literally just '/'?
        // printf("Entered cmd[0] == '/'");
        if (access(cmd, F_OK) == 0){
            *resolved_command = strdup(cmd);
            if (*resolved_command == NULL){
                return 1;
            }
            return 0;
        }
    }
    char* cur_path;
    char* full_path;
    // printf("Path size: %d", path_size);
    for (int i = 0; i < path_size; i++){
        cur_path = *(path+i);
        size_t full_path_len = strlen(cmd) + strlen(cur_path) + 2;
        full_path = (char*) malloc(full_path_len);
        if (full_path == NULL){
            // printf("Full_path is null\n");
            return 1;
        }
        strcpy(full_path, cur_path);
        strcat(full_path, "/");
        strcat(full_path, cmd);
        // printf("\nChecking full path: {%s}\n", full_path);
        // printf("==========================\n");
        if (access(full_path, F_OK) == 0){
            // printf("Access succeeded\n");
            *resolved_command = strdup(full_path);
            // printf("resolved command: {%s}", *resolved_command);
            free(full_path);
            if (*resolved_command == NULL){
                // printf("Strdup to resolved_command failed\n");
                return 1;
            }
            // printf("Strdup to resolved_command succeeded\n");
            return 0;
        } else {
            // printf("Access failed\n");
            free(full_path);
        }
    }
    return 1;
}

int _handle_non_built_in_command(char **parsed_args, char** path, int *path_size)
{

    // handle path resolution
    char* resolved_command = NULL;
    int resolve_command_errored = _resolve_command_path(parsed_args[0], path, *path_size, &resolved_command);

    // printf("Resolve command: %s, errored: %d\n", resolved_command, resolve_command_errored);

    if (resolve_command_errored){
        free(resolved_command);
        return resolve_command_errored;
    }

    int rc = fork();

    if (rc < 0)
    {
        fprintf(stderr, "Fork failed\n"); // debug
        free(resolved_command);
        return 1;
    }
    else if (rc == 0)
    {
        // in child - run exec
        parsed_args[0] = resolved_command;
        execv(parsed_args[0], parsed_args);
        fprintf(stderr, "execv failed: %s\n", strerror(errno));  // uncomment this for debugging

        // Exit, don't return. The child will continue to run the shell loop otherwise.
        exit(1);
    }
    else
    {
        // in parent - wait for child
        wait(NULL);
    }
    free(resolved_command);
    return 0;
}

int _handle_path_cmd(char **path_args, char **path, int *path_size)
{
    // skip 'path'
    ++path_args;
    while (*path_args != NULL)
    {
        if (*path_size >= MAX_STRING_ARRAY_SIZE)
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

int _handle_cd_cmd(char **cd_args, int cd_argc)
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
        ret = _handle_path_cmd(parsed_args, path, path_size);
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
        ret = _handle_cd_cmd(parsed_args, argc);
    }
    else
    {
        ret = _handle_non_built_in_command(parsed_args, path, path_size);
    }

    free(parsed_args);
    return ret;
}

int _inject_bin_into_path(char **path, int *path_size)
{
    char *bin = strdup("/bin");
    if (bin == NULL)
    {
        // printf("bin is null in _injecct\n");
        return 1;
    }
    path[*(path_size)] = bin;
    (*path_size)++;
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
    // printf("path size before: %d\n", path_size);

    if (_inject_bin_into_path(path, &path_size) != 0)
    {
        _free_arr_of_strings(path, path_size);
        return 1;
    }
    // printf("path size after: %d\n", path_size);


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
    //fprintf(stdout, "\n");
    return ret;
}
