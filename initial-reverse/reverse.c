#include <stdio.h>
#include <stdlib.h>

typedef struct Node {
    char* line;
    struct Node* next;
} Node;


void push(Node** stack_top, char* line) {
    // TODO - implement
    return;
}
void pop_and_print(Node** stack_top, FILE* out_fp) {
    // TODO -implement
    return;
}




int main(int argc, char* argv) {
    if (argc > 3) {
        fprintf(stderr, "usage: reverse <input> <output>\n");
        exit(1);
    } 

    FILE* in_fp;
    FILE* out_fp;

    if (argc == 1){
        in_fp = stdin;
        out_fp = stdout;
    } else if (argc == 2){
        in_fp = fopen(argv[1], 'r');
        out_fp = stdout;
        if (!in_fp) {
            fprintf("error: cannot open file %s\n", argv[1]);
            exit(1);
        }
    } else if (argc == 3){
        in_fp = fopen(argv[1], 'r');
        if (!in_fp) {
            fprintf("error: cannot open file %s\n", argv[1]);
            exit(1);
        }
        out_fp = fopen(argv[2], 'w');
        if (!out_fp){
            fprintf("error: cannot open file %s\n", argv[2]);
            fclose(in_fp);
            exit(1);
        }   
    }


    // Read line by line, add to a stack

    Node* stack;

    char* line_buffer = NULL;

    ssize_t buffer_size;
    size_t line_size = 0;


    while ((line_size = getline(&line_buffer, &buffer_size, in_fp)) != 1) {
        push(&stack, line_buffer);
    }

    while(stack != NULL){
        pop_and_print(&stack, out_fp);
    }

    free(line_buffer);

    if (in_fp != stdin){
        fclose(in_fp);
    }
    if (out_fp != stdout){
        fclose(out_fp);
    }

}