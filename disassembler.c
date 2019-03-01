#include <stdio.h>
#include <fcntl.h>
#include <sys/types.h>
#include <sys/uio.h>
#include <unistd.h>
#include <inttypes.h>
#include <stdlib.h>
#include <errno.h>
#include <string.h>

#include "printRoutines.h"

#define ERROR_RETURN -1
#define SUCCESS 0

long int currentAddress;        // index to start reading instructions
FILE *machineCode, *outputFile; // files to read from/write to

/* Checks the address if it is multiple of 8 and 
 * the next 8 bytes if there is an 8-byte number it prints it;
 * otherwise prints the byte number. Then it sets the currentAddress to 
 * next instruction.
 * @BEFORE: set currentAddress to beginning of the instruction.
 */
void printInvalid(FILE *in)
{
    long long quad;
    fseek(in, currentAddress, SEEK_SET);
    if (currentAddress % 8 == 0)
    {
        if (fread(&quad, 8, 1, in) == 1)
        {
            print_quad(outputFile, quad);
            currentAddress = currentAddress + 8;
            // file index is at next instruction
        }
        else
        {
            fseek(in, currentAddress, SEEK_SET);
            int c = fgetc(in); // always valid; its called after getting the byte
            print_byte(outputFile, c);
            currentAddress++;
        }
    }
    else
    {
        fseek(in, currentAddress, SEEK_SET);
        int c = fgetc(in); // always valid; its called after getting the byte
        print_byte(outputFile, c);
        currentAddress++;
        // file index is at next instruction
    }
}

int main(int argc, char **argv)
{
    // Verify that the command line has an appropriate number
    // of arguments

    if (argc < 2 || argc > 4)
    {
        fprintf(stderr, "Usage: %s InputFilename [OutputFilename] [startingOffset]\n", argv[0]);
        return ERROR_RETURN;
    }

    // First argument is the file to read, attempt to open it
    // for reading and verify that the open did occur.
    machineCode = fopen(argv[1], "rb");

    if (machineCode == NULL)
    {
        fprintf(stderr, "Failed to open %s: %s\n", argv[1], strerror(errno));
        return ERROR_RETURN;
    }

    // Second argument is the file to write, attempt to open it for
    // writing and verify that the open did occur. Use standard output
    // if not provided.
    outputFile = argc <= 2 ? stdout : fopen(argv[2], "w");

    if (outputFile == NULL)
    {
        fprintf(stderr, "Failed to open %s: %s\n", argv[2], strerror(errno));
        fclose(machineCode);
        return ERROR_RETURN;
    }

    // If there is a 3rd argument present it is an offset so convert it
    // to a numeric value.
    if (4 == argc)
    {
        errno = 0;
        currentAddress = strtol(argv[3], NULL, 0);
        if (errno != 0)
        {
            perror("Invalid offset on command line");
            fclose(machineCode);
            fclose(outputFile);
            return ERROR_RETURN;
        }
    }

    fprintf(stderr, "Opened %s, starting offset 0x%lX\n", argv[1], currentAddress);
    fprintf(stderr, "Saving output to %s\n", argc <= 2 ? "standard output" : argv[2]);

    // if 3rd argument is 0 or default
    // default is 0 as well
    if (currentAddress == 0)
    {
        // find the first non-zero byte
        while (fgetc(machineCode) == 0)
        {
            currentAddress++;
        }
    }
    // 3rd argument is not 0
    else
    {
        // set the file position of the stream to the provided offset
        // the provided offset is the 3rd argument (currentAddress)
        fseek(machineCode, currentAddress, SEEK_SET);
        // find the first non-zero byte
        while (fgetc(machineCode) == 0)
        {
            currentAddress++;
        }
    }
    // if the given binary file is empty
    // simply do nothing and finish the execution
    if (feof(machineCode) && currentAddress == 0)
    {
        fclose(machineCode);
        fclose(outputFile);
        return SUCCESS;
    }
    // if there is only one byte in the given binary file
    // and if that byte is zero then
    // simly print out zero byte and finish the execution
    if (feof(machineCode) && currentAddress == 1)
    {
        printZeroByte(outputFile, 0);
        fclose(machineCode);
        fclose(outputFile);
        return SUCCESS;
    }
    // if there are more than one zero byte and
    // if there are no non-zero byte
    // then print the position of the last zero byte
    // and also a zero byte indication then
    // finish the execution
    if (feof(machineCode) && currentAddress > 1)
    {
        currentAddress--;
        print_pos(outputFile, currentAddress);
        print_byte(outputFile, 0);
        fclose(machineCode);
        fclose(outputFile);
        return SUCCESS;
    }
    // print out the position of the first non-zero byte
    if (currentAddress != 0)
    {
        print_pos(outputFile, currentAddress);
    }

    // update the file position of the stream to the currentAddress
    // so that we're pointing to the first non-zero byte of machineCode
    fseek(machineCode, currentAddress, SEEK_SET);

    // the byte (character) that we'll be reading
    int theCharacter = 0;

    // halt count
    int haltCount = 0;

    // while it is not the end of the given binary file
    // continue fetching and working on the provided bytes
    while (!(feof(machineCode)))
    {
        theCharacter = fgetc(machineCode);
        currentAddress++;

        // handle the case for zeros
        if (feof(machineCode))
        {
            if (haltCount == 1)
            {
                print_halt(outputFile);
            }
            if (haltCount > 1)
            {
                print_halt(outputFile);
                currentAddress--;
                print_pos(outputFile, currentAddress);
                print_byte(outputFile, 0);
            }
            fclose(machineCode);
            fclose(outputFile);
            return SUCCESS;
        }

        if (theCharacter == 0)
        {
            haltCount++;
        }
        else
        {
            if (haltCount == 1)
            {
                print_halt(outputFile);
                haltCount = 0;
            }
            else if (haltCount > 1)
            {
                print_halt(outputFile);
                fprintf(outputFile, "\n");
                currentAddress--;
                print_pos(outputFile, currentAddress);
                haltCount = 0;
            }
            else
            {
                haltCount = 0;
            }
        }

        // get the function (fn) codes for operations (OP) and
        // conditioanl moves (cmov):
        int iFun;
        if ((0x0f & (theCharacter >> 4)) == 2)
        {
            iFun = theCharacter & 0x0f;
            theCharacter = 0x20;
        }
        if ((0x0f & (theCharacter >> 4)) == 6)
        {
            iFun = theCharacter & 0x0f;
            theCharacter = 0x60;
        }
        if ((0x0f & (theCharacter >> 4)) == 7)
        {
            iFun = theCharacter & 0x0f;
            theCharacter = 0x70;
        }

        int registers;
        int register1;
        int register2;

        switch (theCharacter)
        {
        // 0x00 for halt
        case 0x00:
            // we handle the case for 0x00 above
            break;

        // 0x10 for nop
        case 0x10:
            print_nop(outputFile);
            break;

        // 0x20 for cmov
        case 0x20:
            // we need to fetch the fn (iFun) code
            if ((iFun > 6) || (iFun < 0))
            {
                // it is an invalid instruction, check the address and print byte
                currentAddress--;
                printInvalid(machineCode);
                break;
            }
            // check the registers if iFun and iCode are ok:
            registers = fgetc(machineCode);
            if (feof(machineCode))
            {
                currentAddress--;
                printInvalid(machineCode);
                break;
            }
            // update everytime we successfully read
            currentAddress++;
            register1 = (registers & 0xf0) >> 4;
            register2 = registers & 0x0f;
            // check whether it is valid or not
            if (register1 < 0 || register2 < 0 || register1 > 14 || register2 > 14)
            {
                currentAddress -= 2; // decrement before you call printinvalid
                printInvalid(machineCode);
                break;
            }
            // if it is a valid cmov instruction:
            printTwoBytes(outputFile, 2, iFun, register1, register2);
            break;

        // irmovq
        case 0x30:
            break;

        // rmmov rA, D(rB)
        case 0x40:
            break;

        case 0x50:
            break;

        case 0x60:
            break;

        case 0x70:
            break;

        case 0x80:
            break;

        case 0x90:
            break;
        
        case 0xA0:
            break;
        }

        fclose(machineCode);
        fclose(outputFile);
        return SUCCESS;
    }
}
