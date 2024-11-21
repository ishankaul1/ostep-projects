#include <stdio.h>
#include <stdlib.h>
#include <string.h>

// helper function: search for term in file


int main(int argc, char* argv[]) {
    if (argc == 1) {
        printf("wgrep: searchterm [file ...]\n");
        return 1;
    }

    char* searchTerm = argv[1];
    FILE *fp;

    if (argc == 2) {
        fp = stdin;
    }
    for (int i = 2; i < argc; i++) {
        fp = fopen(argv[i], "r");
        if (fp == NULL) {
            printf("wgrep: cannot open file\n");
            exit(1);
        }

    }
    // assume lines won't be longer than 10k chars
    char lineBuffer[10000];
    while (fgets(lineBuffer, sizeof(lineBuffer), fp) != NULL) {
        if (strstr(lineBuffer, searchTerm) != NULL) {
            printf("%s", lineBuffer);
        }
    }

    if (fp != stdin) {
        fclose(fp);
    }

    return 0;
}