#ifndef DECODE_H
#define DECODE_H
#include <stdio.h>
#include "types.h"
#include <string.h>
typedef struct _DecodeInfo
{
  char *stego_fname;
  FILE *fptr_stego;

  char *dest_fname;
  FILE *fptr_dest;
  char output_fname[50];

} DecodeInfo; 

Status open_file(DecodeInfo *decInfo);
Status read_and_validate_decode_args(char *argv[], DecodeInfo *decInfo);
Status do_decoding(DecodeInfo *decInfo);
Status skip_bmp_header(FILE *fptr_stego);
Status decode_magic_string(FILE *fptr_stego,char *magic_string);
char lsb_to_byte(char *buffer);
int lsb_to_sizee(char *buffer);
Status decode_extn_size(FILE *fptr_stego, int *extn_size);
Status decode_extn(FILE *fptr_stego, char *extn, int extn_size);
Status decode_sec_file_size(FILE *fptr_stego, int *file_size);
Status decode_sec_data(FILE *fptr_stego,FILE *fptr_dest,int file_size);
#endif