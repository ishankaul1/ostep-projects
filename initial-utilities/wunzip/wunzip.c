#include <stdio.h>
#include <stdlib.h>

int process_file(FILE *fp)
{
    if (fp == NULL)
    {
        printf("wunzip: cannot open file\n");
        exit(1);
    }

    unsigned char record[5];

    while (fread(record, 1, 5, fp) == 5)
    {
        // Assumes little-endian
        u_int32_t count = record[0] | (record[1] << 8) | (record[2] << 16) | (record[3] << 24);
        char c = record[4];

        for (int i = 0; i < count; i++)
        {
            putchar(c);
        }
    }
    return 0;
}

int main(int argc, char *argv[])
{
    if (argc == 1)
    {
        printf("wunzip: file1 [file2 ...]\n");
        exit(1);
    }

    FILE *fp;

    for (int i = 1; i < argc; i++)
    {
        fp = fopen(argv[i], "r");
        process_file(fp);
        if (fp != stdin)
        {
            fclose(fp);
        }
    }
}