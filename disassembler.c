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