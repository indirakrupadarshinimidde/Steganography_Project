#include <stdio.h>
#include <stdlib.h>
#include <string.h>
#include "decode.h"
#include "types.h"
#include "encode.h"


Status open_file(DecodeInfo *decInfo)
{
    decInfo->fptr_stego = fopen(decInfo->stego_fname, "rb");
    if (decInfo->fptr_stego == NULL)
    {
        perror("fopen");
        printf("ERROR: Unable to open file %s\n", decInfo->stego_fname);
        return e_failure;
    }

    printf("-------------------------------------------------\n");
    printf("OPEN FILE\n");
    printf("----------------------------------------------------\n");
    printf("Initial Offset : %ld\n", ftell(decInfo->fptr_stego));

    return e_success;
}
Status read_and_validate_decode_args(char *argv[], DecodeInfo *decInfo)
{
    if (argv[2] == NULL)
    {
        printf("Stego file not provided\n");
        return e_failure;
    }

    if (strstr(argv[2], ".bmp") == NULL)
    {
        printf("Invalid stego file\n");
        return e_failure;
    }

    decInfo->stego_fname = argv[2];

    if (argv[3] != NULL)
        decInfo->dest_fname = argv[3];
    else
        decInfo->dest_fname = "decoded";

    return e_success;
}
Status skip_bmp_header(FILE *fptr_stego)
{
    printf("-------------------------------------------------\n");
    printf("SKIP BMP HEADER\n");
    printf("--------------------------------------------------\n");
    printf("Before Skip Offset : %ld\n", ftell(fptr_stego));

    fseek(fptr_stego, 54, SEEK_SET);

    printf("After Skip Offset  : %ld\n", ftell(fptr_stego));
    return e_success;
}


char lsb_to_byte(char *buffer)
{
    char ch = 0;
    for (int i = 0; i < 8; i++)
    {
        ch <<= 1;
        ch |= (buffer[i] & 1);
    }
    return ch;
}

int lsb_to_sizee(char *buffer)
{
    int size = 0;
    for (int i = 0; i < 32; i++)
    {
        size <<= 1;
        size |= (buffer[i] & 1);
    }
    return size;
}
Status decode_magic_string(FILE *fptr_stego, char *magic_string)
{
    char buffer[8];

    printf("-------------------------------------------\n");
    printf("DECODE MAGIC STRING\n");
    printf("---------------------------------------------\n");
    printf("Before Magic Offset : %ld\n", ftell(fptr_stego));

    for (int i = 0; i < strlen(MAGIC_STRING); i++)
    {
        fread(buffer, 1, 8, fptr_stego);
        magic_string[i] = lsb_to_byte(buffer);
    }

    printf("After Magic Offset  : %ld\n", ftell(fptr_stego));

    magic_string[strlen(MAGIC_STRING)] = '\0';

    if (strcmp(magic_string, MAGIC_STRING) != 0)
    {
        printf("Magic string mismatch\n");
        return e_failure;
    }

    printf("Magic string verified: %s\n", magic_string);
    return e_success;
}
Status decode_extn_size(FILE *fptr_stego, int *extn_size)
{
    char buffer[32];

    printf("------------------------------------------\n");
    printf("DECODE EXTENSION SIZE\n");
    printf("----------------------------------\n");
    printf("Before Extn Size Offset : %ld\n", ftell(fptr_stego));

    fread(buffer, 1, 32, fptr_stego);
    *extn_size = lsb_to_sizee(buffer);

    printf("After Extn Size Offset  : %ld\n", ftell(fptr_stego));

    return e_success;
}
Status decode_extn(FILE *fptr_stego, char *extn, int extn_size)
{
    char buffer[8];

    printf("----------------------------------------\n");
    printf("DECODE EXTENSION DATA\n");
    printf("-----------------------------------------\n");
    printf("Before Extn Data Offset : %ld\n", ftell(fptr_stego));

    for (int i = 0; i < extn_size; i++)
    {
        fread(buffer, 1, 8, fptr_stego);
        extn[i] = lsb_to_byte(buffer);
    }
    printf("After Extn Data Offset  : %ld\n", ftell(fptr_stego));
    extn[extn_size] = '\0';

    return e_success;
}
Status decode_sec_file_size(FILE *fptr_stego, int *file_size)
{
    char buffer[32];

    printf("---------------------------------------------\n");
    printf("DECODE SECRET FILE SIZE\n");
    printf("---------------------------------------------\n");
    printf("Before File Size Offset : %ld\n", ftell(fptr_stego));

    fread(buffer, 1, 32, fptr_stego);
    *file_size = lsb_to_sizee(buffer);

    printf("After File Size Offset  : %ld\n", ftell(fptr_stego));

    return e_success;
}
Status decode_sec_data(FILE *fptr_stego, FILE *fptr_dest, int file_size)
{
    char buffer[8];
    char ch;

    printf("----------------------------------------\n");
    printf("DECODE SECRET FILE DATA\n");
    printf("---------------------------------------\n");

    printf("Before Secret Data Offset : %ld\n", ftell(fptr_stego));
    for (int i = 0; i < file_size; i++)
    {
        fread(buffer, 1, 8, fptr_stego);
        ch = lsb_to_byte(buffer);
        fwrite(&ch, 1, 1, fptr_dest);
    }
    printf("After Secret Data Offset  : %ld\n\n", ftell(fptr_stego));

    return e_success;
}
Status do_decoding(DecodeInfo *decInfo)
{
    char magic[10];
    int extn_size;
    char extn[20];
    int file_size;

    if (open_file(decInfo) == e_failure)
        return e_failure;

    skip_bmp_header(decInfo->fptr_stego);

    if (decode_magic_string(decInfo->fptr_stego, magic) == e_failure)
        return e_failure;

    decode_extn_size(decInfo->fptr_stego, &extn_size);
    decode_extn(decInfo->fptr_stego, extn, extn_size);

    strcpy(decInfo->output_fname, decInfo->dest_fname);
    strcat(decInfo->output_fname, extn);

    decInfo->fptr_dest = fopen(decInfo->output_fname, "wb");
    if (decInfo->fptr_dest == NULL)
    {
        printf("Unable to create output file\n");
        return e_failure;
    }

    decode_sec_file_size(decInfo->fptr_stego, &file_size);
    decode_sec_data(decInfo->fptr_stego, decInfo->fptr_dest, file_size);

    printf("*****************************************************\n");
    printf("\033[1;32m=====Decoding completed successfully=====\n\033[0m");
    printf("*****************************************************\n");

    fclose(decInfo->fptr_stego);
    fclose(decInfo->fptr_dest);

    return e_success;
}
