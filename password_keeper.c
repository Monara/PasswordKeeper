/*
===============================================================================
Name        : password_keeper.c
Author      : $ Monika
Copyright   : $(copyright)
Description : Password Keeper, project for C Programming for Smart Systems

October 2019
LPCXpresso1549
===============================================================================
*/

#if defined (__USE_LPCOPEN)
#if defined(NO_BOARD_LIB)
#include "chip.h"
#else
#include "board.h"
#endif
#endif

#include <cr_section_macros.h>
#include <stdio.h>
#include <ctype.h>
#include <stdbool.h>
#include <string.h>
#include <stdlib.h>

#define PWD_FILE "pwd.txt"
#define INPUT_BUFFER_SIZE 64

struct dataItem {
    char *address;
    int address_length;
	char *password;
    int password_length;
};

bool inputIsWebsite(char *input);
void addData(char *mkey);
void printAll(char *mkey);
bool search(char *input, char *mkey);
void encrypt(char *mkey, char *pwd, int pwd_length);
void writeItem(struct dataItem* item, FILE* file);
bool readItem(struct dataItem* item, FILE* file);
void createItem(struct dataItem* item, char *address, char *password);
void freeItem(struct dataItem* item);

int main(void) {

#if defined (__USE_LPCOPEN)
    // Read clock settings and update SystemCoreClock variable
    SystemCoreClockUpdate();
#if !defined(NO_BOARD_LIB)
    // Set up and initialize all required blocks and
    // functions related to the board hardware
    Board_Init();
    // Set the LED to the state of "On"
    Board_LED_Set(0, true);
#endif
#endif

    char buffer[INPUT_BUFFER_SIZE];
    char input[INPUT_BUFFER_SIZE];
    char mkey[INPUT_BUFFER_SIZE];

    printf("This is Password Keeper.\nPlease insert the master key:\n");
    fgets(buffer, INPUT_BUFFER_SIZE, stdin);
    sscanf(buffer, "%s", &mkey);

    for(;;)
    {
    	printf("To search, type in an address (e.g. amazon.com).\n"
    			"To add a new password, type in \"add\".\n"
    			"To print a report of all saved passwords, type in \"all\".\n"
    			"To quit the programme, type in \"quit\".\n");
    	memset(buffer, 0, INPUT_BUFFER_SIZE);  					//clear the buffer in between commands
    	memset(input, 0, INPUT_BUFFER_SIZE);
    	fgets(buffer, INPUT_BUFFER_SIZE, stdin);

    	if (sscanf(buffer, "%s", input) > 0) 	//if nothing typed, for example, goes back to beginning
    	{

			if (strcmp(input, "quit") == 0) 			//if typed quit
			{
				printf("Quitting programme...");
				break;
			}
			else if (strcmp(input, "all") == 0) 		// if to print a report of all
			{
				printAll(mkey);
			}
			else if (strcmp(input, "add") == 0) 		// if to add new data
			{
				addData(mkey);
			}
			else
			{
				if(search(input, mkey) == false)  		//if matching website address is found on file, it will print the password
				{
					if (inputIsWebsite(input) == true)
					{
						printf("No saved passwords for %s found.\n", input); //if dots in input, must be a website
					}
					else
					{
						printf("Command \"%s\" not recognised.\n", input); //if no dots, must be wrong command
					}
				}
			}
		}
    }
		memset(mkey, 0, sizeof(mkey)); //wipe out master key
		return 0;
}

bool inputIsWebsite(char *input)
{
	int inputLength = strlen(input);
	for (int i = 0; i < inputLength; i++)
	{
		if(input[i] == '.' ) 		//if there's a dot in input, user was likely looking for a website
	    {
	    	return true;
	    }
	}
	return false;
}

void createItem(struct dataItem* item, char *address, char *password)
{
	item->password_length = strlen(password);	 		//get length of password, assign
	item->address_length = strlen(address);
	item->address = malloc(item->address_length + 1); 	//plus one for \0
	strcpy(item->address, address);						//copy contents into malloc'ed address
	item->password = malloc(item->password_length + 1);
	strcpy(item->password, password);
}

void freeItem(struct dataItem* item)  //wipe and free
{
	if (item->address != NULL) //null when searching empty file
	{
		memset(item->address, 0, item->address_length); //replace address with zeroes
		free(item->address);
	}
	if (item->password != NULL)
	{
		memset(item->password, 0, item->password_length); //replace password with zeroes
		free(item->password);
	}
}

void writeItem(struct dataItem* item, FILE *file)
{
	fwrite(&item->address_length, sizeof(item->address_length), 1, file); //write address length into file first
	fwrite(item->address, item->address_length, 1, file);					//writes address and tells how long it is
	fwrite(&item->password_length, sizeof(item->password_length), 1, file);
	fwrite(item->password, item->password_length, 1, file);
}

bool readItem(struct dataItem* item, FILE *file)
{
	item->address = NULL;  	//in case of error, because they're freed later
	item->password = NULL;

	if (fread(&item->address_length, sizeof(item->address_length), 1, file) == 0) //read length of address
	{
		return false; //if not read, then following readings won't work: terminate function
	}
	item->address = malloc(item->address_length + 1); //if reading works, allocate the length of address + \0

	if (fread(item->address, item->address_length, 1, file) == 0) //read into address from file
	{
		return false;
	}
	item->address[item->address_length] = '\0';		//string: the final char should be null terminating char

	if (fread(&item->password_length, sizeof(item->password_length), 1, file) == 0)
	{
		return false;
	}
	item->password = malloc(item->password_length + 1);

	if (fread(item->password, item->password_length, 1, file) == 0)
	{
		return false;
	}
	item->password[item->password_length] = '\0';
	return true;
}

void addData(char *mkey)
{
	char buffer[INPUT_BUFFER_SIZE];
	char address[INPUT_BUFFER_SIZE];
	char password[INPUT_BUFFER_SIZE];

	printf("Insert the address (e.g. amazon.com, without \"www\"):\n"); //asks for address
	fgets(buffer, INPUT_BUFFER_SIZE, stdin);
	int a = sscanf(buffer, "%s", address); 	//to verify that it was read and assigned

	if (a > 0) //proceed only with read address
	{

		if(search(address, mkey) == true)	//checking if address isn't already in the file, prints it
		{
			printf("Address already exists.\n");
		}

		else
		{
			printf("Insert the password:\n");		//asks for password
			fgets(buffer, INPUT_BUFFER_SIZE, stdin);
			int b = sscanf(buffer, "%s", password);

			if (a + b < 2)
			{
				printf("Error! Not all data read correctly.\n");
			}
			else
			{
				struct dataItem* addItem = malloc(sizeof(struct dataItem));
				createItem(addItem, address, password);
				encrypt(mkey, addItem->password, addItem->password_length); 	//encrypt the password before writing into file

				FILE *afile = fopen(PWD_FILE, "ab"); //open file to append

				if (afile != NULL)
				{
					writeItem(addItem, afile); //function to write data item into file
					printf("Data added.\n");
				}
				else
				{
					printf("Error opening file!\n");
				}
				fclose(afile);
				freeItem(addItem); //free and wipe address and password
				memset(addItem, 0, sizeof(struct dataItem)); //wipe struct
				free(addItem);		//free struct
			}
		}
	}

	else
	{
		printf("Invalid address.\n");
	}
}

void printAll(char *mkey)
{
	char adr[] = "Address"; 		//for aligning printing columns
	struct dataItem* printItem = malloc(sizeof(struct dataItem));
	FILE *rfile = fopen(PWD_FILE, "rb");	//open file for reading

	if (rfile != NULL)
	{
		printf("%-40s Password \n", adr);

		for(int i= 0; !feof(rfile); i++) //until the end of file, read, decrypt, print, free and wipe
		{
				if (readItem(printItem, rfile) == true)
				{
					encrypt(mkey, printItem->password, printItem->password_length);
					printf("%-40s %s\n", printItem->address, printItem->password);
					freeItem(printItem); //free and wipe address and password
				}
		}
		fclose(rfile);
	}
	else
	{
		printf("Error opening file!");
	}
	memset(printItem, 0, sizeof(struct dataItem)); //wipe struct
	free(printItem);
}

bool search(char *input, char *mkey)
{
	bool addressFound = false;
	struct dataItem* searchItem = malloc(sizeof(struct dataItem));
	FILE *rfile = fopen(PWD_FILE, "rb");	//open file in read mode

	if (rfile != NULL)
	{
		for(int i= 0; !feof(rfile); i++) 	//until the end of file
		{
			readItem(searchItem, rfile); //taking each struct item from file and comparing to input

				if (strcmp(input, searchItem->address) == 0) //if it's a match, decrypts and prints
				{
					encrypt(mkey, searchItem->password, searchItem->password_length);
					printf("%-40s %s\n", searchItem->address, searchItem->password);
					addressFound = true;
					freeItem(searchItem); //wipe memory and free address and password
					break;
				}
			freeItem(searchItem); //wipe memory and free address and password
		}
		fclose(rfile);
	}
	else
	{
		printf("Error opening file!\n");
	}
	memset(searchItem, 0, sizeof(struct dataItem)); //wipe struct
	free(searchItem);		//free struct
	return addressFound;
}

void encrypt(char *mkey, char *pwd, int pwd_length)
{
	int mkey_length = strlen(mkey); 			//length of master key

	for (int i = 0; i < pwd_length; i++)
	{
		pwd[i] = pwd[i]^mkey[i%mkey_length];  //XOR. if master key is shorter than password, repeats it
	}
}




