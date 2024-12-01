#include <stdio.h>
#include <stdlib.h>

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

int main(int argc, char *argv[])
{
    if (argc == 1)
    {
        printf("wzip: file1 [file2 ...]\n");
        exit(1);
    }

    FILE *fp;

    int currentCount = 1;
    char prev = '\0';
    char cur;

    size_t size_out;
    char* fileBuffer;

    for (int i = 1; i < argc; i++)
    {
        // TODO - figure out how to write a helper function with this nasty cross-file implementation
        fp = fopen(argv[i], "r");

        if (fp == NULL)
        {
            printf("wzip: cannot open file\n");
            exit(1);
        }

        fileBuffer = read_file(fp, &size_out);

        if (fileBuffer == NULL)
        {
            printf("wzip: cannot open file\n");
            exit(1);
        }


        for (int j = 0; j < size_out; j++)
        {
            cur = fileBuffer[j];
            if (prev == '\0')
            {
                prev = cur;
            }
            else if (cur != prev)
            {
                fwrite(&currentCount, sizeof(int), 1, stdout);
                fwrite(&prev, sizeof(char), 1, stdout);
                prev = cur;
                currentCount = 1;
            }
            else
            {
                currentCount++;
            }
        }

        if (fp != stdin)
        {
            fclose(fp);
        }
    }

    fwrite(&currentCount, sizeof(int), 1, stdout);
    fwrite(&prev, sizeof(char), 1, stdout);
    free(fileBuffer);
    return 0;
}