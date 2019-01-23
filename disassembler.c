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

long int currentAddress; // index to start reading instructions
FILE *machineCode, *outputFile; // files to read from/write to

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
      
   switch(c){
          // halt
        case 0x00 :
            // do not do anything because we do it when we read the byte/
            // this is handled differently according to the spec
         break;
         
         // nop
        case 0x10 :
            printByte(outputFile, 1);
            
         break;
         
         // cmov
        case 0x20 :
            // get the fn code
            if((fn) >6 || (fn)<0){
                // invalid instruction, check the address and print byte
                currentAddress--;
                printInvalid(outputFile);
                break;
            }
            
            // check registers if code is OK
            registers = fgetc(machineCode);
            if( feof(machineCode) ) { // break the loop if end of file
                currentAddress -= 1; // decrement before you call printinvalid
                printInvalid(machineCode);
                 break ;
            }
            currentAddress++; // update every time you read
            r1 = (registers & 0xf0)>>4;
            r2 = registers & 0x0f;
            
            if(r1 < 0 || r2 < 0 || r1 > 14 || r2 > 14){
                currentAddress -= 2; // decrement before you call printinvalid
                printInvalid(machineCode);
                break;
            }
   
            // valid instruction
            printTwoBytes(outputFile, 2, fn, r1, r2);
         break;
         
         // irmov
         case 0x30 :
            // check registers if code is OK
            registers = fgetc(machineCode);
            if( feof(machineCode) ) { // break the loop if end of file
                currentAddress -= 1; // decrement before you call printinvalid
                printInvalid(machineCode);
                 break ;
            }
            
            currentAddress++; // update every time you read
            r1 = (registers & 0xf0)>>4;
            r2 = registers & 0x0f;
           
            if(r1 != 15 || r2 < 0 || r2 > 14){
                currentAddress -= 2; // decrement before you call printinvalid
                printInvalid(machineCode);
                break;
            }
            
            // check value if code is OK
            if(fread(&value, 8, 1, machineCode) != 1){
                currentAddress = currentAddress -2;
                printInvalid(outputFile);
                break;
            }
            // update every time you read
            currentAddress = currentAddress + 8;
            // valid instruction
            printTenBytes(outputFile, 3, r1, r2, value);
             
         break;
         
         // rmmov rA, D(rB)
         case 0x40 :
            // check registers if code is OK
            registers = fgetc(machineCode);
            if( feof(machineCode) ) { // break the loop if end of file
                currentAddress -= 1; // decrement before you call printinvalid
                printInvalid(machineCode);
                 break ;
            }
            currentAddress++; // update every time you read
            r1 = (registers & 0xf0)>>4;
            r2 = registers & 0x0f;
            
            if(r1 <0 || r1>14 || r2 < 0 || r2 > 14){
                currentAddress -= 2; // decrement before you call printinvalid
                printInvalid(machineCode);
                break;
            }
            
            
            // check offset if code is OK
            if(fread(&value, 8, 1, machineCode) != 1){
                currentAddress = currentAddress -2;
                printInvalid(outputFile);
                break;
            }
            // update every time you read
            currentAddress +=8;
            
            // valid instruction
            printTenBytes(outputFile, 4, r1, r2, value);
         break;
         
         // mrmov D(rA), rA
         case 0x50 :
            // check registers if code is OK
            registers = fgetc(machineCode);
            if( feof(machineCode) ) { // break the loop if end of file
                currentAddress -= 1; // decrement before you call printinvalid
                printInvalid(machineCode);
                 break ;
            }
            
            currentAddress++; // update every time you read
            r1 = (registers & 0xf0)>>4;
            r2 = registers & 0x0f;
            
            if(r1 <0 || r1>14 || r2 < 0 || r2 > 14){
                currentAddress -= 2; // decrement before you call printinvalid
                printInvalid(machineCode);
                break;
            }
            
            // check offset if code is OK
            if(fread(&value, 8, 1, machineCode) != 1){
                currentAddress = currentAddress -2;
                printInvalid(outputFile);
                break;
            }
            // update every time you read
            currentAddress +=8;
            // valid instruction
            printTenBytes(outputFile, 5, r1, r2, value);
         break;
   
         // OPfn rA, rB // TODO
         case 0x60 :
            // get the fn code
            if((fn) >6 || (fn)<0){
                // invalid instruction, check the address and print byte
                currentAddress--;
                printInvalid(outputFile);
                break;
            }
            
            // check registers if code is OK
            registers = fgetc(machineCode);
            if( feof(machineCode) ) { // break the loop if end of file
                currentAddress -= 1; // decrement before you call printinvalid
                printInvalid(machineCode);
                 break ;
            }
            currentAddress++; // update every time you read
            r1 = (registers & 0xf0)>>4;
            r2 = registers & 0x0f;
            
            if(r1 < 0 || r2 < 0 || r1 > 14 || r2 > 14){
                currentAddress -= 2; // decrement before you call printinvalid
                printInvalid(machineCode);
                break;
            }
            
            // valid instruction
            printTwoBytes(outputFile, 6, fn, r1, r2);
         break;
         
         // jXX DEST
         case 0x70 :  // TODO 
            // get the fn code
            if((fn) >6 || (fn)<0){
                // invalid instruction, check the address and print byte
                currentAddress--;
                printInvalid(outputFile);
                break;
            }
            
            // check dest if code is OK
            if(fread(&value, 8, 1, machineCode) != 1){
                currentAddress--;
                printInvalid(outputFile);
                break;
            }
            // update every time you read
            currentAddress +=8;
            
            // valid instruction
            printNineBytes(outputFile, 7, fn, value);
         break;
         
         // call Dest
         case 0x80 :  
            // check dest if code is OK
            if(fread(&value, 8, 1, machineCode) != 1){
                currentAddress--;
                printInvalid(outputFile);
                break;
            }
            // update every time you read
            currentAddress +=8;
            
            // valid instruction
            printNineBytes(outputFile, 8, 0, value);
         break;
         
         // ret
         case 0x90 :
         printByte(outputFile, 2);
         break;
         
         // pushq rA
         case 0xA0 :
            // check registers if code is OK
            registers = fgetc(machineCode);
            if( feof(machineCode) ) { // break the loop if end of file
                currentAddress -= 1; // decrement before you call printinvalid
                printInvalid(machineCode);
                 break ;
            }
            currentAddress++; // update every time you read
            r1 = (registers & 0xf0)>>4;
            r2 = registers & 0x0f;
            
            if(r1 < 0 || r1 > 14 || r2 != 15){
                currentAddress -= 2; // decrement before you call printinvalid
                printInvalid(machineCode);
                break;
            }
            
            // valid instruction
            printTwoBytes(outputFile, 10, -1, r1, r2);
         break;
         
         // popq rA
         case 0xB0 :
             // check registers if code is OK
            registers = fgetc(machineCode);
            if( feof(machineCode) ) { // break the loop if end of file
                currentAddress -= 1; // decrement before you call printinvalid
                printInvalid(machineCode);
                 break ;
            }
            currentAddress++; // update every time you read
            r1 = (registers & 0xf0)>>4;
            r2 = registers & 0x0f;
            
            if(r1 < 0 || r1 > 14 || r2 != 15){
                currentAddress -= 2; // decrement before you call printinvalid
                printInvalid(machineCode);
                break;
            }
            
            // valid instruction
            printTwoBytes(outputFile, 11, -1, r1, r2);
         break;
         
         // HANDLE any other case
        default :
            currentAddress = currentAddress - 1;
            printInvalid(machineCode);
            break;
       }
   }



  fclose(machineCode);
  fclose(outputFile);
  return SUCCESS;
}
