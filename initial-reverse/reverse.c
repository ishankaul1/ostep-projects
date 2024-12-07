#include <stdio.h>
#include <stdlib.h>
#include <string.h>
#include <sys/stat.h>


typedef struct Node
{
    char *line;
    struct Node *next;
} Node;

void push(Node **stack_top, char *line)
{
    Node *new_node = malloc(sizeof(Node));
    if (!new_node)
    {
        fprintf(stderr, "malloc failed\n");
        exit(1);
    }
    new_node->line = strdup(line);
    if (!new_node->line)
    {
        fprintf(stderr, "malloc failed\n");
        free(new_node);
        exit(1);
    }
    new_node->next = *stack_top;
    *stack_top = new_node;
}
void pop_and_print(Node **stack_top, FILE *out_fp)
{
    Node *temp_top = *stack_top;
    fputs(temp_top->line, out_fp);
    *stack_top = temp_top->next;
    free(temp_top->line);
    free(temp_top);
}

int are_hard_linked(FILE* fp_1, FILE* fp_2){
    struct stat stat1;
    struct stat stat2;

    int fd1 = fileno(fp_1);
    int fd2 = fileno(fp_2);

    if (fstat(fd1, &stat1) < 0 || fstat(fd2,  &stat2) < 0) {
        fprintf(stderr, "failed to get file stats\n");
    }

    return (stat1.st_ino == stat2.st_ino);

}

int main(int argc, char **argv)
{
    if (argc > 3)
    {
        fprintf(stderr, "usage: reverse <input> <output>\n");
        exit(1);
    }

    FILE *in_fp;
    FILE *out_fp;

    if (argc == 1)
    {
        in_fp = stdin;
        out_fp = stdout;
    }
    else if (argc == 2)
    {
        in_fp = fopen(argv[1], "r");
        out_fp = stdout;
        if (!in_fp)
        {
            fprintf(stderr, "reverse: cannot open file '%s'\n", argv[1]);
            exit(1);
        }
    }
    else if (argc == 3)
    {
        if (strcmp(argv[1], argv[2]) == 0)
        {
            fprintf(stderr, "reverse: input and output file must differ\n");
            exit(1);
        }
        in_fp = fopen(argv[1], "r");
        if (!in_fp)
        {
            fprintf(stderr, "reverse: cannot open file '%s'\n", argv[1]);
            exit(1);
        }
        out_fp = fopen(argv[2], "w");
        if (!out_fp)
        {
            fprintf(stderr, "reverse: cannot open file '%s'\n", argv[2]);
            fclose(in_fp);
            exit(1);
        }
        if (are_hard_linked(in_fp, out_fp)){
            fprintf(stderr,"reverse: input and output file must differ\n");
            exit(1);
        }
    }

    // Read line by line, add to a stack

    Node *stack = NULL;

    char *line_buffer = NULL;

    size_t buffer_size;
    ssize_t line_size = 0;

    while ((line_size = getline(&line_buffer, &buffer_size, in_fp)) != -1)
    {
        push(&stack, line_buffer);
    }

    while (stack != NULL)
    {
        pop_and_print(&stack, out_fp);
    }

    free(line_buffer);

    if (in_fp != stdin)
    {
        fclose(in_fp);
    }
    if (out_fp != stdout)
    {
        fclose(out_fp);
    }
}