#include <stdio.h>
#include "decode.h"
#include "types.h"
#include<string.h>
#include "common.h"

/*Defining function to get file names and validate
 *Input : stego.bmp file
 *output: data hidden in stego.bmp image is decoded in output.txt file
 *return value:e_success or e_failure, on file errors
 */


//defining a function to open files for decoding
Status open_decode_files(DecodeInfo *decInfo)
{
	decInfo->fptr_stego_image=fopen(decInfo->stego_image_fname,"r");   //opeinig stego image file in read mode

	if (decInfo->fptr_stego_image == NULL)              //validating if error exists while opening file
	{
		perror("fopen");                   //perror is a function used to print just a message as a normal printf
		fprintf(stderr,"ERROR: Unable to open file %s\n",decInfo->stego_image_fname);
		return e_failure;
	}
	return e_success;
}

//definition of a function to read and validate arguments for decoding
Status read_and_validate_decode_args(char *argv[], DecodeInfo *decInfo)
{
	if (argv[2] != NULL)
	{
		if (strstr(argv[2],".bmp"))                                       //checking if third argument is .bmp file or not
		{
			decInfo->stego_image_fname=argv[2];                           //assign argv[2] to pointer stego_image_fname
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
	if (argv[3] != NULL)                                      //checking if fourth argument is NULL or not
	{
		strcpy(decInfo->output_fname,argv[3]);                            //if not NULL strcpy argv[3] to output_fname array
	}
	else
	{
		strcpy(decInfo->output_fname,"output");                   //else strcpy "output" to output_fname
	}
	return e_success;
}


//defining a function to decode magic string
Status decode_magic_string(char *magic_string, DecodeInfo *decInfo)
{
	//set the file pointer to 54 in source image
	fseek(decInfo->fptr_stego_image,54,SEEK_SET);
	char magic_data[(strlen(magic_string)+1)];                            //declare char array magic_data of size of string length of magic string +1

	//call decode_data_from_image to decode magic string from the stego image
	decode_data_from_image(magic_data,strlen(magic_string),decInfo->fptr_stego_image);

	magic_data[strlen(magic_string)]='\0';                               //assign NULL value to array
	if (strcmp(magic_data,MAGIC_STRING)==0)                              //compare magic string with the decoded magic string
	{
		return e_success;
	}
	else
	{
		return e_failure;
	}
}

//defining a generic function ---> to decode data from the stego image
Status decode_data_from_image(char *data, long size, FILE *fptr_stego_image)
{
	int i;
	char str[8];                                                        //declare char array str of size 8
	for (i=0;i<size;i++)
	{
		fread(str,8,1,fptr_stego_image);               //read 8 bytes from the stego image and store it in an str

		decode_byte_from_lsb(&data[i],str);           //call the function decode_byte_from_lsb to decode ecah character
	}
}

//defining a generic function ---> to decode each chacter from stego image
Status decode_byte_from_lsb(char *data, char *image_buffer)
{
	int i;
	char ch=0x00;                                                       //declare char ch and assign 0x00 to it
	for (i=0;i<8;i++)
	{
		ch = ch | ((image_buffer[i] & 1) << i);                          //bitwise operation to get each character
	}
	*data=ch;

}

//defining a function to decode secret file extn size from the stego image
Status decode_secret_file_extnsize(DecodeInfo *decInfo)
{	
	char str[32];                                                        //declare char array of size 32
	fread(str,32,1,decInfo->fptr_stego_image);  
//	printf("the data readd<from file is %d\n", str);                         //read 32 bytes from stego image and store it in str
	decode_size_from_lsb(&decInfo->extn_size,str);                       //call function to decode size from the stego image
	printf("the extn size is %d\n",decInfo->extn_size );
	return e_success;
}

//definin a generic funstion ---> to decode size from the stego image
Status decode_size_from_lsb (int *data, char *image_buffer)
{
	int i;
	char ch=0x00;                                                        //declare char ch and assign 0x00 to it
	for (i=0;i<32;i++)
	{
		ch = ch | ((image_buffer[i] & 1) << i);                          //bitwise operation to get size
	}
	*data=ch;
}

//defining a function to decode secret file extn from stego image
Status decode_secret_file_extn(DecodeInfo *decInfo)
{
	char extn_data[decInfo->extn_size];                                  //declare char array extn_data of size extn_size

	//call data_from_image to decode secret file extn from the stego image
	decode_data_from_image(extn_data,decInfo->extn_size,decInfo->fptr_stego_image);

	//strcat the output_fname and extn_data and assign it to output_file_fname
	decInfo->output_file_fname=strcat(decInfo->output_fname,extn_data);

	//open the file output_file_fname in write mode
	decInfo->fptr_output_file=fopen(decInfo->output_file_fname,"w");

	//check for file opened or not
	if (decInfo->fptr_output_file == NULL)
	{
		perror("fopen");
		fprintf(stderr,"ERROR: Unable to open file %s\n",decInfo->output_fname);
		return e_failure;
	}
	return e_success;
}

//defining a function to decode secret file size from the stego image
Status decode_secret_file_size(DecodeInfo *decInfo)
{
	char str[32];                                                       //declare char array str of size 32

	fread(str,32,1,decInfo->fptr_stego_image);                          //read 32 bytes from the stego image and store it in str

	decode_size_from_lsb(&decInfo->secret_size,str);              //call decode_size_from_lsb to decode size from the stego image
	return e_success;
}

//defining a function to decode secret data from the stego image
Status decode_secret_file_data(DecodeInfo *decInfo)
{
	char secret_data[decInfo->secret_size];                             //declare char array secret_data od size secret_size

	//call decode_data_from_image to decode secret data from the stego image
	decode_data_from_image(secret_data,decInfo->secret_size,decInfo->fptr_stego_image);

	//write the secret_data into output file
	fwrite(secret_data,decInfo->secret_size,1,decInfo->fptr_output_file);
	return e_success;
}


//defining a function to do decoding
Status do_decoding(DecodeInfo *decInfo)
{
	//to check files opened successfully or not
	if (open_decode_files(decInfo)==e_success)
	{
		printf("INFO: opening required files for decoding\n");
		printf("INFO: Done\n");

		//to check whether the magic string decode successfully or not
		if (decode_magic_string(MAGIC_STRING,decInfo)==e_success)
		{
			printf("INFO: Decoding magic string \n");
		    printf("INFO: Done\n");

			//to check whether the extn size  decode or not
			if (decode_secret_file_extnsize(decInfo)==e_success)
			{
				printf("INFO: Decoding extn size\n");
		        printf("INFO: Done\n");

				//to check whether the extn data is decode or not
				if (decode_secret_file_extn(decInfo)==e_success)
				{
					printf("INFO: Decoding extn  data\n");
		            printf("INFO: Done\n");
					//to check secret file size is decode or not
					if (decode_secret_file_size(decInfo)==e_success)
					{
						printf("INFO: Decoding secret size\n");
		                printf("INFO: Done\n");

						//to check secret file data is decode or not
						if (decode_secret_file_data(decInfo)==e_success)
						{
							printf("INFO: Decoding secret file data\n");
		                    printf("INFO: Done\n");
						}
						else
						{
							printf("Decoding secret file data failure\n");
							return e_failure;
						}
					}
					else
					{
						printf("Decoding secret size failure\n");
						return e_failure;
					}
				}
				else
				{
					printf("Decoding extn data failure\n");
					return e_failure;
				}
			}
			else
			{
				printf("Decoding extn size failure\n");
				return e_failure;
			}
		}
		else
		{
			printf("Decoding magic string failure\n");
			return e_failure;
		}
	}
	else
	{
		printf("open files for decoding failure\n");
		return e_failure;
	}
	return e_success;
}
