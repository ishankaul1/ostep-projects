#include <stdio.h>
#include <stdlib.h>

// TODO - make this a utility in this repo; seems to be useful already
char *read_file(FILE *fp, size_t *size_out)
{
    fseek(fp, 0, SEEK_END);
    long size = ftell(fp);
    *size_out = size;
    fseek(fp, 0, SEEK_SET);
    char *buffer = malloc(size);

    if (!buffer)
        return NULL;

    size_t read = fread(buffer, 1, size, fp);
    if (read != size)
    {
        free(buffer);
        return NULL;
    }

    return buffer;
}

char *process_file(FILE *fp)
{
    if (fp == NULL)
    {
        printf("wunzip: cannot open file\n");
        exit(1);
    }

    size_t file_length;
    // need to free; gets malloced
    char *fileBuffer = read_file(fp, &file_length);

    if (fileBuffer == NULL)
    {
        printf("wunzip: cannot open file\n");
        exit(1);
    }

    for (int i = 0; i < file_length; i++) {
        // parse out int and char

        // emit char int times
    }

    free(fileBuffer);
}

int main(int argc, char *argv[])
{
    if (argc == 1)
    {
        printf("wunzip: file1 [file2 ...]");
    }

    FILE *fp;

    for (int i = 1; i < argc; i++)
    {
        fp = fopen(argv[i], "r");
    }
}