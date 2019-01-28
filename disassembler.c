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

  // Comment or delete the following lines and this comment before
  // handing in your final version.
  //if (currentAddress) printPosition(outputFile, currentAddress);
  //printInstruction(outputFile);
  //printInstruction(outputFile);

  
  
   int c = 0;
   if(currentAddress == 0){ // 3rd argument is default or 0
    while (c == 0) { // find the first non-zero byte
        c = fgetc(machineCode);
        currentAddress++;
    }
   }else{ // 3rd argument is not zero
       fseek(machineCode, currentAddress, SEEK_SET);
       while (c == 0) { // find the first non-zero byte
        c = fgetc(machineCode);
        currentAddress++;
        if( feof(machineCode) ){
            printPos(outputFile, currentAddress-1);
            printByte(outputFile, 0);
        }
    }
   }
   
   if (currentAddress != 1){ // check if its the first byte of the file
       printPosSpace(outputFile, currentAddress-1);
   }
   
   // first non-zero byte is found.
   // set the currentAddress to the address of first non zero byte and
   // update the file pointer to that address before you start 
   // reading instructions.  
   currentAddress -= 1;
   fseek(machineCode, currentAddress, SEEK_SET);
   
   int haltCount = 0;
   
   
   while(!(feof(machineCode))){
       
       fseek(machineCode, currentAddress, SEEK_SET); // start from the currentaddr
       c = fgetc(machineCode);
       currentAddress++; // every time you read update the address
        if( feof(machineCode) ) { // break the loop if end of file
            
            if(haltCount == 1){
                printByte(outputFile, 0);
            }
            
            if (haltCount > 1) {
                printPos(outputFile, currentAddress-1);
                printByte(outputFile, 0);
            }
            haltCount = 0;
            break ;
        }
        
   int value;
   int registers;
   int fn;
   int r1;
   int r2;
  
   
   // count the number of consecutive halt instructions
   if(c == 0){
       haltCount = haltCount + 1;
   }else{ // handle more than one halt followed by non-zero inst
       

       if(haltCount == 1){
           printByte(outputFile, 0);
           haltCount = 0;
       }
       else if(haltCount > 1){
           printByte(outputFile, 0);
           fprintf(outputFile,"\n");
           printPos(outputFile, currentAddress-1);
           haltCount = 0;
       } else{haltCount = 0;
       }
       
   }
   
   
   
   // get the function codes for OP and conditional instructions
   if((0x0f&(c>>4)) == 2){
       fn = c & 0x0f;
       c = 0x20;
   }
   if((0x0f&(c>>4)) == 6){
       fn = c & 0x0f;
       c = 0x60;
   }
   if((0x0f&(c>>4)) == 7){
       fn = c & 0x0f;
       c = 0x70;
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
