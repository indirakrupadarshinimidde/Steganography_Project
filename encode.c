#include<stdio.h>
#include "encode.h"
#include <string.h>
#include "types.h"

 //./a.out -e beautiful.bmp secret.txt stego.bmp

/* Get image size
 * Input: Image file ptr
 * Output: width * height * bytes per pixel (3 in our case)
 * Description: In BMP Image, width is stored in offset 18,
 * and height after that. size is 4 bytes
 */
uint get_image_size_for_bmp(FILE *fptr_image)
{
    uint width, height;
    // Seek to 18th byte
    fseek(fptr_image, 18, SEEK_SET);

    // Read the width (an int)
    fread(&width, sizeof(int), 1, fptr_image);
    printf("width = %u\n", width);

    // Read the height (an int)
    fread(&height, sizeof(int), 1, fptr_image);
    printf("height = %u\n", height);

    // Return image capacity
    return width * height * 3;
}

/* 
 * Get File pointers for i/p and o/p files
 * Inputs: Src Image file, Secret file and
 * Stego Image file
 * Output: FILE pointer for above files
 * Return Value: e_success or e_failure, on file errors
 */
Status open_files(EncodeInfo *encInfo)
{
    // Src Image file
    encInfo->fptr_src_image = fopen(encInfo->src_image_fname, "rb");
    // Do Error handling
    if (encInfo->fptr_src_image == NULL)
    {
    	perror("fopen");
    	fprintf(stderr, "ERROR: Unable to open file %s\n", encInfo->src_image_fname);

    	return e_failure;
    }

    // Secret file
    encInfo->fptr_secret = fopen(encInfo->secret_fname, "rb");
    // Do Error handling
    if (encInfo->fptr_secret == NULL)
    {
    	perror("fopen");
    	fprintf(stderr, "ERROR: Unable to open file %s\n", encInfo->secret_fname);

    	return e_failure;
    }

    // Stego Image file
    encInfo->fptr_stego_image = fopen(encInfo->stego_image_fname, "wb");
    // Do Error handling
    if (encInfo->fptr_stego_image == NULL)
    {
    	perror("fopen");
    	fprintf(stderr, "ERROR: Unable to open file %s\n", encInfo->stego_image_fname);

    	return e_failure;
    }

    // No failure return e_success
    return e_success;
}

Status read_and_validate_encode_args(char *argv[], EncodeInfo *encInfo)
{
  if(argv[1] == NULL)
  {
    printf(".bmp file not passed\n"); 
    return e_failure;
  }
   if(strstr(argv[2], ".bmp" ) == NULL)
   {
    printf("Invalid image file name\n");
    return e_failure;
   }
   encInfo->src_image_fname = argv[2];
   if(argv[3] == NULL)
   {
    printf(".txt file is not passed\n");
    return e_failure;
   }
   if(strstr(argv[3],".txt") == NULL)
   {
    printf("Invalid sec file name\n");
    return e_failure;
   }
   encInfo->secret_fname = argv[3];

   if(argv[4] == NULL)
   {
    encInfo->stego_image_fname = "stego.bmp";
   }
   else
   {
    encInfo->stego_image_fname = argv[4];
   }

   char *chr;
  chr = strchr(encInfo->secret_fname,'.');
   
    if( chr != NULL)
   {
    strcpy(encInfo->extn_secret_file,chr);
   }
    printf("%s\n",encInfo->extn_secret_file);
        return e_success;
    /*
        1.check argv[2] == NULL
        yes-> print .bmp file not passed
                return failure
        2.check strstr(argv[2], ".bmp") == NULL
        yes->   print invalid image file name
                return failure
    
        3. encInfo->src_image_fname = argv[2];
        4. check argv[3] == NULL
        yes-> print .txt file not passed
                return failure
        5. check strstr(argv[3], ".txt") == NULL
        yes-> print invalid sec file name
                return failure
        6. encInfo->sec_fname = argv[3]
        7. check argv[4] == NULL
        yes-> encInfo->stego_image_fname = "stego.bmp"
        no->  validate and store to encInfo->stego_image_fname = argv[4]
        8. copy sec file extn to encInfo->extn_sec_file array
            1. char *chr = strchar(encInfo->sec_fname, '.')
            2. strcpy(encInfo->extn_sec_file, chr);
        9. return success

        */
}
Status do_encoding(EncodeInfo *encInfo)
{
   int ret =(open_files(encInfo));
    if(ret == e_failure)
    {
      printf("open file failed");
      return e_failure;
    }
    ret = check_capacity(encInfo);
     if(ret == e_failure)
     {
    printf("Insufficient capacity\n");
    return e_failure;
     }

    //printf("before copying header offset is :%d",ftell(fptr));

       copy_bmp_header(encInfo->fptr_src_image, encInfo->fptr_stego_image);
    
       encode_magic_string(MAGIC_STRING, encInfo);
       printf("Magic string is: %s\n", MAGIC_STRING);
       printf("Length of Magic String: %lu\n", strlen(MAGIC_STRING));
       encode_secret_file_extn(strlen(encInfo->extn_secret_file), encInfo);
       encode_secret_file_extn_data(encInfo);
       encode_secret_file_size(encInfo->size_secret_file, encInfo);
       encode_secret_file_data(encInfo);
       copy_remaining_img_data(encInfo->fptr_src_image,
                            encInfo->fptr_stego_image);

       return e_success;
 }
 Status check_capacity(EncodeInfo *encInfo)
{
  int total_size_img;//syz of src file
  int total_req_size;//req size no.of bytes to node
  int secret_size;//syz of secret fyl data
                                                                   
   total_size_img = get_image_size_for_bmp(encInfo->fptr_src_image);
   rewind(encInfo->fptr_src_image);

   fseek(encInfo->fptr_secret, 0, SEEK_END);  
    
   secret_size = ftell(encInfo->fptr_secret);
   rewind(encInfo->fptr_secret);

   encInfo->size_secret_file = secret_size;
   rewind(encInfo->fptr_secret);
   
   total_req_size = (strlen(MAGIC_STRING)+
                     sizeof(int) +
                     strlen(encInfo->extn_secret_file)+
                     sizeof(int)+
                     secret_size)*8+54;
                    
                     if(total_req_size <= total_size_img)
                     {
                        return e_success;
                     }
                     else{
                        return e_failure;
                     }
                
}
//     /*
//         1.calculate no. of bytes needed for encoding from src file
//             count = (magic_str_len +
//             extn_size(int) + 
//             extn_len + 
//             file_size(int) + 
//             file_data_len ) * 8 + 54

//         2. check count <= src_file_size
//         yes-> return sucess
//         no-> return failure
//     */
// 
Status copy_bmp_header(FILE *fptr_src_image, FILE *fptr_dest_image)
 {
    printf("--------------------------------------------\n"); 
    printf("Copy BMP Header\n");
    printf("--------------------------------------------\n"); 
    printf("Before copy SRC offset position is  : %ld\n", ftell(fptr_src_image));
    printf("Before copy STEGO offset position is : %ld\n", ftell(fptr_dest_image));
       char temp[55];

         fread(temp, 54, 1, fptr_src_image);
         fwrite(temp, 54, 1, fptr_dest_image);
    printf("After copy SRC offset  offset position is : %ld\n", ftell(fptr_src_image));
    printf("After copy STEGO offset position is : %ld\n", ftell(fptr_dest_image));
         
        return e_success;
//     /*
//         char temp[55];
//         fread(fptr_src_image, 54, 1, temp);

//         fwrite(fptr_dest_image, 54, 1, temp);

//         return success
//     */
 }
 Status encode_magic_string(const char *magic_string, EncodeInfo *encInfo)
 {
         printf("---------------------------------------------\n"); 
         printf(" Encode Magic String\n");
          printf("--------------------------------------------\n"); 
    printf("Before encoding Magic String Offest position is : %ld\n", ftell(encInfo->fptr_src_image));
    printf("Before encoding Magic String Offest position is:%ld\n", ftell(encInfo->fptr_stego_image));

  for(int i=0; magic_string[i] != '\0';i++)
  {
        char temp[8];
       fread(temp, 1, 8, encInfo->fptr_src_image);

       encode_byte_to_lsb(magic_string[i], temp);

       fwrite(temp, 1, 8, encInfo->fptr_stego_image);
        
  }
  printf("Offest position after encoding Magic String:%ld\n",ftell(encInfo->fptr_src_image));
  printf("Offest position after encoding Magic String:%ld\n",ftell(encInfo->fptr_stego_image));
  return e_success;
 }
 

Status encode_secret_file_extn(int file_extn_size, EncodeInfo *encInfo)
{
        char temp_buffer[32];
        printf("------------------------------------------------\n"); 
         printf(" Encode Extension Size \n");
         printf("-----------------------------------------------\n"); 
    printf("Before EXT SIZE SRC offset position is  : %ld\n", ftell(encInfo->fptr_src_image));
    printf("Before EXT SIZE STEGO offset position is: %ld\n", ftell(encInfo->fptr_stego_image));
        fread(temp_buffer, 1,32,encInfo->fptr_src_image);

       encode_size_to_lsb_int(file_extn_size, temp_buffer);

        fwrite(temp_buffer, 1, 32,encInfo->fptr_stego_image);
      
    printf("After EXT SIZE SRC offset  position is : %ld\n", ftell(encInfo->fptr_src_image));
    printf("After EXT SIZE STEGO offset position is: %ld\n", ftell(encInfo->fptr_stego_image));
        return e_success;
}
Status encode_secret_file_extn_data(EncodeInfo *encInfo)
{
    int len = strlen(encInfo->extn_secret_file);
     printf("------------------------------------------------------\n"); 
     printf(" Encode Extension Data\n");
     printf("-------------------------------------------------------\n"); 
    printf("Before EXT DATA SRC offset position is  : %ld\n", ftell(encInfo->fptr_src_image));
    printf("Before EXT DATA STEGO offset position is: %ld\n", ftell(encInfo->fptr_stego_image));

    for(int i = 0; i < len; i++)
    {
        char temp[8];

        fread(temp, 1, 8, encInfo->fptr_src_image);

        encode_byte_to_lsb(encInfo->extn_secret_file[i], temp);

        fwrite(temp, 1, 8, encInfo->fptr_stego_image);
    }
     printf("After EXT SIZE SRC offset position is   : %ld\n", ftell(encInfo->fptr_src_image));
    printf("After EXT SIZE STEGO offset position is : %ld\n", ftell(encInfo->fptr_stego_image));
    return e_success;
}

Status encode_secret_file_size(int file_size, EncodeInfo *encInfo)
{
        char temp_buffer[32];
        printf("-----------------------------------------------------\n");
        printf("Encode secret file size:\n");
        printf("------------------------------------------------------\n");
        printf("Offest position before encoding size:%ld\n",ftell(encInfo->fptr_src_image));
        printf("Offest position before encoding size:%ld\n",ftell(encInfo->fptr_stego_image));
        fread(temp_buffer, 1, 32, encInfo->fptr_src_image);

        encode_size_to_lsb_int(file_size, temp_buffer);

        fwrite(temp_buffer, 1, 32, encInfo->fptr_stego_image);
        printf("Offest position after encoding size:%ld\n",ftell(encInfo->fptr_src_image));
        printf("Offest position after encoding size:%ld\n",ftell(encInfo->fptr_stego_image));

        return e_success;
}

Status encode_secret_file_data(EncodeInfo *encInfo)
{
    printf("-----------------------------------------------------\n");
    printf("Encode Secret File Data\n");
    printf("-----------------------------------------------------\n");

    printf("Before Data Src offset position   : %ld\n", ftell(encInfo->fptr_src_image));
    printf("Before Data stego offset position : %ld\n", ftell(encInfo->fptr_stego_image));
     
    char ch;
    char temp_buffer[8];

    rewind(encInfo->fptr_secret);

    while(fread(&ch, 1, 1, encInfo->fptr_secret) == 1)
    {
        fread(temp_buffer, 1, 8,encInfo->fptr_src_image);

        encode_byte_to_lsb(ch, temp_buffer);
        fwrite(temp_buffer, 1, 8,encInfo->fptr_stego_image);
    }
    printf("After Data Src offset  position  : %ld\n", ftell(encInfo->fptr_src_image));
    printf("After Data stego offset position : %ld\n", ftell(encInfo->fptr_stego_image));

    return e_success;
}
//encode_magic_string function call
 Status encode_byte_to_lsb(char data, char *image_buffer)
{
   for(int i=0; i<8;i++)
   {
    image_buffer[i] = image_buffer[i] & 0xFE;

    image_buffer[i] = image_buffer[i] | (data >> (7-i) & 1);
   }
   return e_success;
 
}

//encode_secret_file_extn_size function
Status encode_size_to_lsb_int(int data, char *image_buffer)
{
   for(int i=0;i<32;i++)
   {
          image_buffer[i] &= 0xFE;  
          image_buffer[i] |=((data>> (31 - i)) & 1);
   }
   return e_success;
}
//encode_secret_file_size function call
Status encode_size_to_lsb_long(long data, char *image_buffer)
{
    for(int i = 0; i < 32; i++)
    {
        image_buffer[i] &= 0xFE;
        image_buffer[i] |= ((data >> (31 - i)) & 1);
    }
    return e_success;
 }
Status copy_remaining_img_data(FILE *fptr_src, FILE *fptr_dest)
{
    printf("---------------------------------------------------\n");
    printf("Copy Remaining Image Data\n");
    printf("------------------------------------------------------\n");

    printf("Before COPY Src offset  position : %ld\n", ftell(fptr_src));
    printf("Before COPY Stego offset position : %ld\n", ftell(fptr_dest));

    char ch;

    while (fread(&ch, 1, 1, fptr_src) == 1)
    {
        fwrite(&ch, 1, 1, fptr_dest);
    }
 
    printf("After copying Src offset position is: %ld\n", ftell(fptr_src));
    printf("After copying Stego offset position is: %ld\n\n", ftell(fptr_dest));
    printf("******************************************************\n");
    printf("\033[1;32m=====Encoding completed successfully=====\n\033[0m");
    printf("*******************************************************\n");
    return e_success;  
}
 