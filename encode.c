//encode.c
#include <stdio.h> //header file
#include<string.h>
#include "encode.h"
#include "types.h"
#include "common.h"
//OperationType check_operation_type(char *argv[]);
//Status read_and_validate_encode_args(char *argv[], EncodeInfo *encInfo);
/* Function Definitions */

//definition of the function to get the file size
uint get_file_size(FILE *fptr) //unsinged int fucntion 
{
	fseek(fptr,0,SEEK_END); //set the file pointer to end position
	return ftell(fptr); //returning the end position value which is the size of the file
}
/*
 * Get File pointers for i/p and o/p files
 * Inputs: Src Image file, Secret file and
 * Stego Image file
 * Output: FILE pointer for above files
 * Return Value: e_success or e_failure, on file errors
 */

/* Get File pointers for i/p and o/p files */
Status open_files(EncodeInfo *encInfo) 
{
	// Src Image file
	encInfo->fptr_src_image = fopen(encInfo->src_image_fname, "r");
	// Do Error handling
	if (encInfo->fptr_src_image == NULL)
	{
		perror("fopen");
		fprintf(stderr, "ERROR: Unable to open file %s\n", encInfo->src_image_fname);
		return e_failure;
	}
	// Secret file
	encInfo->fptr_secret = fopen(encInfo->secret_fname, "r");
	// Do Error handling
	if (encInfo->fptr_secret == NULL)
	{
		perror("fopen");
		fprintf(stderr, "ERROR: Unable to open file %s\n", encInfo->secret_fname);
		return e_failure;
	}
	// Stego Image file
	encInfo->fptr_stego_image = fopen(encInfo->stego_image_fname, "w");
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
//definition of a function to read and validate encode arguments
Status read_and_validate_encode_args(char *argv[], EncodeInfo *encInfo)
{
	char *ptr;
	if (argv[2] != NULL)  //check if 3rd CLA is present
	{
		if (strstr(argv[2],".bmp")) //checking if the third argument is .bmp file or not
		{
			encInfo->src_image_fname = argv[2]; //assigning the argv[2] to pointer src_image_fname
		}
		else
		{
			return e_failure;
		}
	}
	else
	{
		return e_failure;
	}
	if (argv[3] != NULL) //check if 4th CLA is present
	{
		if (ptr=strchr(argv[3],'.'))   //checking if the 4th argument is .txt or not
		{
			strcpy(encInfo->extn_secret_file,ptr);   //assign extension to array extn_secret_file
			encInfo->secret_fname=argv[3];  //assign argv[3] to pointer secret_fname
		}
		else
		{
			return e_failure;
		}
	}
	else
	{
		return e_failure;
	}
	if (argv[4] == NULL)  //checking if the 5th argument is passed or not
	{
		encInfo->stego_image_fname="stego.bmp"; //if 5th argument is not passed assign stego.bmp to pointer stego_image_fname
	}
	else
	{
		if (strstr(argv[4],".bmp")) //chech if the CLA passed is .bmp 
		{
			encInfo->stego_image_fname=argv[4];   //if 5th  argument is passed assign same name to pointer stego_image_fname
		}
		else
		{
			return e_failure;
		}
	}
	return e_success;
}
/* check capacity */
Status check_capacity(EncodeInfo *encInfo) 
{
	encInfo->image_capacity=get_image_size_for_bmp(encInfo->fptr_src_image); //getting image capacity by calling the function get_image_size_for_bmp
	encInfo->size_secret_file=get_file_size(encInfo->fptr_secret);//getting size of secret file by calling the function get_file_size
	//checking if the required image capacity is enough to hold the secret data
	if(encInfo->image_capacity>(54+(strlen(MAGIC_STRING)*8)+32+(strlen(encInfo->extn_secret_file)*8)+32+encInfo->size_secret_file*8)) //check if size of image is gr
	{
		return e_success;
	}
	else
	{
		return e_failure;
	}
}
/* Copy bmp image header */
Status copy_bmp_header(FILE *fptr_src_image, FILE *fptr_dest_image)
{
	char str[54];   //declaring an char array str with size 54
	fseek(fptr_src_image,0,SEEK_SET);  //setting file pointer to 1st character
	fread(str,54,1,fptr_src_image);    //read 54 bytes header from the source bmp image to str
	fwrite(str,54,1,fptr_dest_image);  //write 54 byte header from the str to destination bmp image
	return e_success;
}

/* Store Magic String */
Status encode_magic_string(char *magic_string, EncodeInfo *encInfo)
{
	fseek(encInfo->fptr_src_image,54,SEEK_SET);  //setting the file pointer to 54 position in source image

	//each element of magic string encoded one after other by calling function encode_data_to_image
	encode_data_to_image(magic_string,strlen(magic_string),encInfo->fptr_src_image,encInfo->fptr_stego_image);
	return e_success;
}

//definition of a function to encode secret file extension size
Status encode_secret_file_extnsize(int extn_size, EncodeInfo *encInfo)
{
	char str [32]; //declaring the char array str of size 32
	//read 32 bytes from source image and encoded to lsb and writing encoded secret file extension size into destination image
	fread (str,32,1,encInfo->fptr_src_image);
	encode_size_to_lsb(extn_size,str);
	fwrite(str,32,1,encInfo->fptr_stego_image);
	return e_success;
}

//definition of a function encode_secret_file_size
Status encode_secret_file_size(long file_size, EncodeInfo *encInfo)
{
	char str[32]; //declare char array str of size 32

	//reading 32 bytes for once from source image and encoded to lsb and writing encoded secret file size into destination image
	fread(str,32,1,encInfo->fptr_src_image);
	encode_size_to_lsb(file_size,str);
	fwrite(str,32,1,encInfo->fptr_stego_image);
	return e_success;
}

//definition of a function encode_secret_file_extn
Status encode_secret_file_extn(char *file_extn, EncodeInfo *encInfo)
{
	//each byte of secret file extension encoded one after other by calling function encode_data_to_image
	encode_data_to_image(file_extn,strlen(file_extn),encInfo->fptr_src_image,encInfo->fptr_stego_image);
	return e_success;
}

//defining function encode_secret_file_data
Status encode_secret_file_data(EncodeInfo *encInfo)
{
	//set file pointer of secret file to first chracter
	fseek(encInfo->fptr_secret,0,SEEK_SET);  //set file pointer of secret file to first chracter
	char str[encInfo->size_secret_file];   //declare char array of size of secret file size

	//read the secret file data and store it an array str
	fread (str,encInfo->size_secret_file,1,encInfo->fptr_secret);

	//encode secret file data by calling the function encode_data_to_image
	encode_data_to_image(str,encInfo->size_secret_file,encInfo->fptr_src_image,encInfo->fptr_stego_image);
	return e_success;
}

//defining generic function ---> encode_data_to_image
Status encode_data_to_image(char *data, int size, FILE *fptr_src_image, FILE *fptr_stego_image)
{
	int i;
	char image_buf[8];   //declare char array image_buf of size 8
	for (i=0;i<size;i++)
	{
		fread(image_buf,8,1,fptr_src_image);    //read the 8 bytes from the source image and store in image_buf
		encode_byte_to_lsb(data[i],image_buf);  //encode byte by byte from the source image by function call encode_byte_to_lsb
		fwrite(image_buf,8,1,fptr_stego_image); //write the 8 bytes from the image_buf to stego_image
	}
}

//defining generic function --->  encode_byte_to_lsb
Status encode_byte_to_lsb (char data, char *image_buffer)
{
	int i;

	for (i=0;i<8;i++)     //each byte of data requires 8byte to make it as lsb
	{
		//below is bitwise operation to perform encode data
		image_buffer[i]=image_buffer[i] & 0xFE;
		image_buffer[i]=image_buffer[i] | ((data & (1 << i)) >> i);
	}
}

//defining generic function ---> encode_size_to_lsb
Status encode_size_to_lsb (int data, char *image_buffer)
{
	int i;
	for (i=0;i<32;i++)     //ecah size requires 32 bytes to amke it as lsb
	{
		//below is bitwise operation to perform encode size
		image_buffer[i]=image_buffer[i] & 0xFE;
		image_buffer[i]=image_buffer[i] | ((data & (1 << i)) >> i);
	}
}


//defining the function to perform encoding
Status do_encoding(EncodeInfo *encInfo)
{
	//to check file opened successfully or not
	if (open_files(encInfo)==e_success)
	{
		printf("INFO: opening  required files for encoding\n");
		printf("INFO: Done\n");

		//to check whether image has capacity to hold the secret data
		if (check_capacity(encInfo)==e_success)
		{
			printf("INFO: Capacity Check of beautiful.bmp to handel secret.txt\n");
		    printf("INFO: Done\n");

			//to check whether source file header copied successfully to stego or not
			if (copy_bmp_header(encInfo->fptr_src_image,encInfo->fptr_stego_image)==e_success)
			{
				printf("INFO: Copying Image Header\n");
		        printf("INFO: Done\n");

				//to check encode magic string is success or not
				if (encode_magic_string(MAGIC_STRING,encInfo)==e_success)
				{
					printf("INFO: Encoding magic string\n");
		            printf("INFO: Done\n");

					//to check encode secret file extnsize is success or not
					if (encode_secret_file_extnsize(strlen(encInfo->extn_secret_file),encInfo)==e_success)
					{
						printf("INFO: Encoding extn size\n");
		                printf("INFO: Done\n");

						//to check encode secret file extn success or not
						if (encode_secret_file_extn(encInfo->extn_secret_file,encInfo)==e_success)
						{
							printf("INFO: Encoding extn data \n");
		                    printf("INFO: Done\n");

							//to check encode secret file size success or not
							if (encode_secret_file_size(encInfo->size_secret_file,encInfo)==e_success)
							{
								printf("INFO: Encoding secret file size\n");
		                        printf("INFO: Done\n");

								//to check encode secret file data success or not
								if (encode_secret_file_data(encInfo)==e_success)
								{
									printf("INFO: Encoding secret file data \n");
		                            printf("INFO: Done\n");

									//to check remaining data is copied to stego image or not
									if (copy_remaining_img_data(encInfo->fptr_src_image,encInfo->fptr_stego_image)==e_success)
									{
										printf("INFO: Copying remaining data\n");
		                                printf("INFO: Done\n");
									}
									//coping of remaining data is failed 
									else
									{
										printf("Failed to copy remaining data\n");
										return e_failure;
									}
								}
								else
								{   //encdoing of scerect file data was failed 
									printf("Encode secret file data failure\n");
									return e_failure;
								}
							}
							else
							{ //encoding the secret file size failed 
								printf("Encode secret file size failure\n");
								return e_failure;
							}
						}
						else
						{ //encoding extn data failed 
							printf("Encode extn data failure\n");
							return e_failure;
						}
					}
					else
					{   //encoding extn size is failed 
						printf("Encode extn size failure\n");
						return e_failure;
					}
				}
				else
				{   //encoding the magic string failed 
					printf("Encode magic string failure\n");
					return e_failure;
				}
			}
			else
			{ //coping the header of source file is failed 
				printf("BMP header copy failure\n");
				return e_failure;
			}
		}
		else
		{  //capacity check of bmp image was failed
			printf("Capacity checking failure\n");
			return e_failure;
		}
	}
	else
	{ //opening of required files for encoding was failed 
		printf("opening files for encoding failure\n");
		return e_failure;
	}
	return e_success; //if all chekc was sucess return e_success
}

//defining function copy_remaining_img_data
Status copy_remaining_img_data (FILE *fptr_src, FILE *fptr_dest)
{
	char ch;        //declaring the char ch variable

	//each byte read one ater other from source image and sent to destination image till it reaches EOF
	while ((fread(&ch,sizeof(char),1,fptr_src))>0)
	{
		fwrite(&ch,sizeof(char),1,fptr_dest);
	}
	return e_success;
}

// unsigned int get_image_size_for_bmp(FILE *fptr_image)
// {
// 	fseek(fptr_image, 0, SEEK_END);
// 	unsigned int size = ftell(fptr_image);
// 	fseek(fptr_image, 0, SEEK_SET);
// 	return size;
// }