\#include <stdio.h>
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

long int currentAddress; // index to start reading instructions
FILE *machineCode, *outputFile; // files to read from/write to

/* Checks the address if it is multiple of 8 and the next 8 bytes if there is 
 * an 8-byte number it prints it;
 * otherwise prints the byte number. Then it sets the currentAddress to 
 * next instruction.
 * @BEFORE: set currentAddress to beginning of the instruction.
 */
void printInvalid(FILE *in){
    long long quad;
    fseek(in, currentAddress, SEEK_SET);
    if (currentAddress % 8 == 0) {
        if(fread(&quad, 8, 1, in) == 1){
            printQuadValue(outputFile, quad);
            currentAddress = currentAddress + 8;
            // file index is at next instruction
        }
        else{
        fseek(in, currentAddress, SEEK_SET);
        int c = fgetc(in); // always valid; its called after getting the byte
        printByteValue(outputFile, c);
        currentAddress = currentAddress + 1;
        }
    }   
    else{
        fseek(in, currentAddress, SEEK_SET);
        int c = fgetc(in); // always valid; its called after getting the byte
        printByteValue(outputFile, c);
        currentAddress = currentAddress + 1;
        // file index is at next instruction
    } 
}



int main(int argc, char **argv) {  
  // Verify that the command line has an appropriate number
  // of arguments

  if (argc < 2 || argc > 4) {
    fprintf(stderr, "Usage: %s InputFilename [OutputFilename] [startingOffset]\n", argv[0]);
    return ERROR_RETURN;
  }

  // First argument is the file to read, attempt to open it 
  // for reading and verify that the open did occur.
  machineCode = fopen(argv[1], "rb");

  if (machineCode == NULL) {
    fprintf(stderr, "Failed to open %s: %s\n", argv[1], strerror(errno));
    return ERROR_RETURN;
  }

  // Second argument is the file to write, attempt to open it for
  // writing and verify that the open did occur. Use standard output
  // if not provided.
  outputFile = argc <= 2 ? stdout : fopen(argv[2], "w");
  
  if (outputFile == NULL) {
    fprintf(stderr, "Failed to open %s: %s\n", argv[2], strerror(errno));
    fclose(machineCode);
    return ERROR_RETURN;
  }

  // If there is a 3rd argument present it is an offset so convert it
  // to a numeric value.
  if (4 == argc) {
    errno = 0;
    currentAddress = strtol(argv[3], NULL, 0);
    if (errno != 0) {
      perror("Invalid offset on command line");
      fclose(machineCode);
      fclose(outputFile);
      return ERROR_RETURN;
    }
  }

  fprintf(stderr, "Opened %s, starting offset 0x%lX\n", argv[1], currentAddress);
  fprintf(stderr, "Saving output to %s\n", argc <= 2 ? "standard output" : argv[2]);

  if(currentAddress == 0){
      while(fgetc(machineCode) == 0){
          currentAddress++;
      }
  }else{
      fseek(machineCode, currentAddress, SEEK_SET);
      while(fgetc(machineCode) == 0){
          currentAddress++;
      }
  }

  if(feof(machineCode) && currentAddress == 0){
      fclose(machineCode);
      fclose(outputFile);
      return SUCCESS;
  }

  if(feof(machineCode) && currentAddress == 1){
      printZeroByte(outputFile, 0);
      fclose(machineCode);
      fclose(outputFile);
      return SUCCESS;
  }

  if(feof(machineCode) && currentAddress > 1){
      currentAddress--;
      print_pos(outputFile, currentAddress);
      print_byte(outputFile, 0);
      fclose(machineCode);
      fclose(outputFile);
      return SUCCESS;
  }

  if(currentAddress != 0){
      print_pos(outputFile, currentAddress);
  }

  // update the file position of the stream to the currentAddress
  // so that we're pointing to the first non-zero byte of machineCode
  fseek(machineCode, currentAddress, SEEK_SET);

  // the byte (character) that we'll be reading
  int theCharacter = 0;

  // halt count
  int haltCount = 0;

  while(!(feof(machineCode))){
      theCharacter = fgetc(machineCode);
      currentAddress++;

      if(feof(machineCode)){
          if(haltCount == 1){
              print_halt(outputFile);
          }
          if(haltCount > 1){
              print_halt(outputFile);
              currentAddress--;
              print_pos(outputFile, currentAddress);
              print_byte(outputFile, 0);
          }
          fclose(machineCode);
          fclose(outputFile);
          return SUCCESS;
      }

      if(theCharacter == 0){
          haltCount++;
      }
      else{
          if(haltCount == 1){
              print_halt(outputFile);
              haltCount = 0;
          }
          else if(haltCount > 1){
              print_halt(outputFile);
              fprintf(outputFile, "\n");
              currentAddress--;
              print_pos(outputFile, currentAddress);
              haltCount = 0;
          }
          else{
              haltCount = 0;
          }
      }

      int iFun;

      if((0x0f & (theCharacter >> 4)) == 2){
          iFun = theCharacter & 0x0f;
          theCharacter = 0x20;
      }
      if((0x0f & (theCharacter >> 4)) == 6){
          iFun = theCharacter & 0x0f;
          theCharacter = 0x60;
      }
      if((0x0f & (theCharacter >> 4)) == 7){
          iFun = theCharacter & 0x0f;
          theCharacter = 0x70;
      }

      switch(theCharacter){
          case 0x00 :
            // we handle it above
            break;
          case 0




      }
  }
  fclose(machineCode);
  fclose(outputFile);
  return SUCCESS;
}
