//main.c
#include <stdio.h> //header files
#include <string.h>
#include "encode.h"
#include "decode.h"
#include "types.h"

int main(int argc, char *argv[])  //main function
{
	if (argc < 2)  //check if number of aruguments are greater then 2
	{
		printf("Encoding:./a.out -e ‹.bmp file> ‹.txt file> (output file]\n"); //print error message if the condtion is wrong
		printf("Decoding:./a.out -d <.bmp file> [output file]\n");
		return 1; //return 1 
	}

	OperationType option; 
	switch(argv[1][1]) //switch case to check which operation to be done encode or decode
	{
		case 'e':                   //case of encode
			option = e_encode;
			break;
		case 'd':                   //case of decode
			option = e_decode;
			break;
		default:                   //default case
			printf("Unsupported option\n"); //if we enter default case print this error message
			return 1;
	}

	if (option == e_encode)  //check if option is equal to e_encode
	{
		//printf("## Encoding Procedure Started ##\n");
		EncodeInfo encInfo; 
		if (read_and_validate_encode_args(argv,&encInfo) == e_success) //check if function read and validate encode return is equal to e_success		
		{
			printf("## Encoding Procedure Started ##\n"); //display message
			printf("Read and validate arguments for encoding success\n");  //dsiplay message
			if (do_encoding(&encInfo)==e_success) //check if de_encoding fucntion is done successfully 
			{
				printf("## Encoding Done Successfully ##\n"); //display message 
			} 
			else 
			{
				printf("## Encoding failure ##\n"); //display message 
			}
		}
		else
		{
			printf("Encoding: ./a.out -e <.bmp file> ‹.txt file> [output file]\n"); //error message when  required aruguments are not given in format
		}
	}
	else if (option == e_decode)  //check if the opertaion to be done e_decode
	{
		DecodeInfo decInfo;
		if(read_and_validate_decode_args(argv,&decInfo) == e_success) //check if vaildation of decode CLA is success
		{
			printf("## Decoding Procedure Started ##\n"); //display message 
			printf("Read and validate arguments for decoding success\n"); //display message 
			if (do_decoding(&decInfo)==e_success) //check if do_decoding  
			{
				printf("## Decoding Done Successfully ##\n");
			} 
			else 
			{
				printf("## Decoding secret data failure ##\n");
			}
		}
		else
		{
			printf("Decoding:./a.out -d <.bmp file> [output file]\n");
		}
	}

	return 0;
}

OperationType check_operation_type(char *argv[])  //defining function to check the operation type
{
	if (argv[1] != NULL)
	{
		if (!(strcmp(argv[1],"-e"))) //if argv[1] contains -e then return e_encode
		{
			return e_encode;
		}
		else if (!(strcmp(argv[1],"-d"))) //if argv[1] contains -d then return e_decode
		{
			return e_decode; //return e_decode
		}
		else
		{
			return e_unsupported; //if the condtions are wrong return e_unsupported 
		}
	}
	else
	{
		return e_unsupported; 
	}
}

/* Function Definitions */
/* Get image size
 * Input: Image file ptr
 * Output: width * height * bytes per pixel (3 in our case)
 * Description: In BMP Image, width is stored in offset 18,
 * and height after that. size is 4 bytes
 */
uint get_image_size_for_bmp(FILE *fptr_image) //unsinged int fucuntion definition
{
	uint width, height; //declare the unsigned int variables
	// Seek to 18th byte
	fseek(fptr_image, 18, SEEK_SET); //set the postion of fptr_image to 18 position
	// Read the width (an int)
	fread(&width, sizeof(int), 1, fptr_image); //read the content from fptr_image and store at width
	printf("width of bmp image = %u\n", width); //display message to print the width of image 
	// Read the height (an int)
	fread(&height, sizeof(int), 1, fptr_image); //read the content of fptr_image and store at the height
	printf("height of bmp image = %u\n", height); //display message to print the height 
	// Return image capacity
	return width * height * 3; //finally return the arthimatic operation of width*height*3 which gives the size of image
}