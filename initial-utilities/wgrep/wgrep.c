#include <stdio.h>
#include <stdlib.h>
#include <string.h>

int process_file(FILE *fp, char *searchTerm)
{
    if (fp == NULL)
    {
        printf("wgrep: cannot open file\n");
        exit(1);
    }

    size_t len = 0;
    ssize_t read;

    char *lineBuffer = NULL;
    while ((read = getline(&lineBuffer, &len, fp)) != -1)
    {
        if (strstr(lineBuffer, searchTerm) != NULL)
        {
            printf("%s", lineBuffer);
        }
    }

    free(lineBuffer);
    if (fp != stdin)
    {
        fclose(fp);
    }
    return 0;
}


int main(int argc, char *argv[])
{
    if (argc == 1)
    {
        printf("wgrep: searchterm [file ...]\n");
        return 1;
    }

    char *searchTerm = argv[1];
    FILE *fp;

    if (argc == 2)
    {
        fp = stdin;
        process_file(fp, searchTerm);
    }
    else
    {
        for (int i = 2; i < argc; i++)
        {
            fp = fopen(argv[i], "r");
            process_file(fp, searchTerm);
        }
    }

    return 0;
}