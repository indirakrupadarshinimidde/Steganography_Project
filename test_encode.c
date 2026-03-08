//main file
#include <stdio.h>
#include<string.h>
#include "encode.h"
#include "decode.h"
#include "types.h"

int main(int argc, char *argv[])
{
    //here argv[] is array of character pointer
    EncodeInfo encInfo;
    DecodeInfo decInfo;

    int ret= check_operation_type(argc, argv);
     
    if(ret == e_unsupported)
    {
        printf("Invalid arguements\n");
        return e_unsupported;
    }

    if(ret == e_encode)
    {
        //encoding 
        ret = read_and_validate_encode_args(argv, &encInfo);
       if(ret == e_failure)
       {
        printf("invalid no.of args\n");//if this condition true it terminates
        return e_failure;
       }
       //do encoding fun call
      ret= do_encoding(&encInfo);
      if(ret == e_failure)
      {
        printf("encoding failed");
        return e_failure;
      }

    }
    if(ret == e_decode)
    {
        //decoding
       printf("Entered Decode Mode\n");  
        if(read_and_validate_decode_args(argv, &decInfo) == e_failure)
    {
        printf("Invalid decode arguments\n");
        return e_failure;
    }

    if(do_decoding(&decInfo) == e_failure)
    {
        printf("Decoding failed\n");
        return e_failure;
    }
}
    }



    /*
    1.cpy header
    2.encode magic str
    3.encode
    */

//DecodeInfo decInfo;   
OperationType check_operation_type(int argc, char *argv[])
{
    if(argc <2)
    {
        return e_unsupported;
    }
    if(strcmp(argv[1],"-e") == 0)
    {
        return e_encode;
    }
    if(strcmp(argv[1],"-d") == 0)
    {
        return e_decode;
    }
    else
    {
       return e_unsupported;
    }
    /*
    1.check argc[1]==NULL
    yes->return e_unsupported
    2.(strcmp(argv[1],"-e")==0)
    yes->return e_encode
    3.(strcmp(argv[1],"-d")==0)
    yes->return e_decode
    4. return e_unsupported

    */
}