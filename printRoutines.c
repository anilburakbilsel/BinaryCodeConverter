#include <stdio.h>
#include <unistd.h>
#include "printRoutines.h"

// the followings are the global variables

int ret = 0; // the RETURN

char *pushq = "pushq";
char *popq = "popq";
char *rrmovq = "rrmovq";

char *stringCMove;     // cMovXX
char *stringCjump;     // jXX
char *stringOperation; // OPq

char *register1; //rA
char *register2; //rB

char *letter; /// USE THIS WHEN rA,rB needs to be a letter :A B C D E F
char *letter2;

// this function identifies the appropriate naming for the register
char *r_Switch(int reg)
{
    char *ret;
    switch (reg)
    {
    case 0:
        ret = "%rax";
        break;
    case 1:
        ret = "%rcx";
        break;
    case 2:
        ret = "%rdx";
        break;
    case 3:
        ret = "%rbx";
        break;
    case 4:
        ret = "%rsp";
        break;
    case 5:
        ret = "%rbp";
        break;
    case 6:
        ret = "%rsi";
        break;
    case 7:
        ret = "%rdi";
        break;
    case 8:
        ret = "%r8";
        break;
    case 9:
        ret = "%r9";
        break;
    case 10:
        ret = "%r10";
        break;
    case 11:
        ret = "%r11";
        break;
    case 12:
        ret = "%r12";
        break;
    case 13:
        ret = "%r13";
        break;
    case 14:
        ret = "%r14";
        break;
    case 15:
        ret = "F"; // TODO
        break;
    }
    return ret;
}

char *letter_Switch(int reg)
{
    switch (reg)
    {
    case 10:
        letter = "A";
        break;
    case 11:
        letter = "B";
        break;
    case 12:
        letter = "C";
        break;
    case 13:
        letter = "D";
        break;
    case 14:
        letter = "E";
        break;
    case 15:
        letter = "F";
        break;
    }
    return letter;
}

char *operation_Switch(int reg)
{
    char *ret;
    switch (reg)
    {
    case 0:
        ret = "addq";
        break;
    case 1:
        ret = "subq";
        break;
    case 2:
        ret = "andq";
        break;
    case 3:
        ret = "xorq";
        break;
    case 4:
        ret = "mulq";
        break;
    case 5:
        ret = "divq";
        break;
    case 6:
        ret = "modq";
        break;
    }
    return ret;
}

char *printCondMove_Switch(int fn)
{
    char *ret;
    switch (fn)
    {
    case 1:
        ret = "cmovle";
        break;
    case 2:
        ret = "cmovl";
        break;
    case 3:
        ret = "cmove";
        break;
    case 4:
        ret = "cmovne";
        break;
    case 5:
        ret = "cmovge";
        break;
    case 6:
        ret = "cmovg";
    }
    return ret;
}

/* %-8s will print out the string left justified and padded out to a total of 8 characters */
/* it is done to have a nicer output */
void print_byte(FILE *out, int val)
{
    fprintf(out, "    %-8s0x%x    # 0X%x\n", ".byte", val, val);
}

void printZeroByte(FILE *out, int val)
{
    fprintf(out, "%-8s0x%x    # %02X\n", ".byte", val, val);
}

void print_pos(FILE *out, int pos)
{
    fprintf(out, ".pos 0x%x\n", pos);
}

void print_position(FILE *out, int pos)
{
    fprintf(out, "    %-8s 0x%x\n", ".pos", pos);
}

void print_halt(FILE *out)
{
    fprintf(out, "    %-8s  # %-22s\n", "halt", "00");
}

void print_nop(FILE *out)
{
    fprintf(out, "    %-8s  # %-22s\n", "nop", "10");
}

void print_quad(FILE *out, int val)
{
    fprintf(out, "    %-8s0x%x    # 0X%x\n", ".quad", val, val);
}

int printTwoBytes(FILE *out, int code, int fn, int reg1, int reg2)
{
    switch (code)
    {
    case 2: //rrmovq rA, rB   --ex: 20AB  , 2012
        // rrmovq (not the conditional move)
        if (fn == 0)
        {
            // identify the registers using the helper function
            register1 = r_Switch(reg1); // string of reg1 , includes rax, rbx ... r10 r11 r12 r13 r14, F
            register2 = r_Switch(reg2); // string of reg2

            //if rA,rB are LETTERS:   20AB-------------------------
            if (reg2 >= 10 && reg2 < 16 && reg1 >= 10 && reg1 < 16)
            {
                letter = letter_Switch(reg1);
                letter2 = letter_Switch(reg2);
                ret = fprintf(out, "    %-8s %s, %s  # %d%d%s%s\n", rrmovq, register1, register2, code, 0, letter, letter2);
                break;
            }
            if (reg2 >= 10 && reg2 < 16)
            {
                letter = letter_Switch(reg2);
                ret = fprintf(out, "    %-8s %s, %s  # %d%d%d%s\n", rrmovq, register1, register2, code, 0, reg1, letter);
                break;
            }
            if (reg1 >= 10 && reg1 < 16)
            {
                letter = letter_Switch(reg1);
                ret = fprintf(out, "    %-8s %s, %s  # %d%d%s%d\n", rrmovq, register1, register2, code, 0, letter, reg2);
                break;
            }
            if (reg1 <= 10 && reg2 <= 10)
            {
                ret = fprintf(out, "    %-8s %s, %s  # %d%d%d%d\n", rrmovq, register1, register2, code, fn, reg1, reg2);
                break;
            }
        }
        else
        {                                             // fn != 0  --- cmovqXX rA, rB         is one of : l, le, e , ne ,ge ,g
            stringCMove = (printCondMove_Switch(fn)); // prints: "cmovXX"
            register1 = r_Switch(reg1);
            register2 = r_Switch(reg2);

            //if rA,rB are LETTERS:   20AB-------------------------
            if (reg2 >= 10 && reg2 < 16 && reg1 >= 10 && reg1 < 16)
            {
                letter = letter_Switch(reg1);
                letter2 = letter_Switch(reg2);
                ret = fprintf(out, "    %-8s %s, %s  # %d%d%s%s\n", stringCMove, register1, register2, code, 0, letter, letter2);
                break;
            }
            if (reg2 >= 10 && reg2 < 16)
            {
                letter = letter_Switch(reg2);
                ret = fprintf(out, "    %-8s %s, %s  # %d%d%d%s\n", stringCMove, register1, register2, code, 0, reg1, letter);
                break;
            }
            if (reg1 >= 10 && reg1 < 16)
            {
                letter = letter_Switch(reg1);
                ret = fprintf(out, "    %-8s %s, %s  # %d%d%s%d\n", stringCMove, register1, register2, code, 0, letter, reg2);
                break;
            }
            if (reg1 <= 10 && reg2 <= 10)
            {
                ret = fprintf(out, "    %-8s %s, %s  # %d%d%d%d\n", stringCMove, register1, register2, code, fn, reg1, reg2);
                break;
            }
            else
            {
                ret = fprintf(out, "    %-8s %s, %s  # %d%d%d%d\n", stringCMove, register1, register2, code, fn, reg1, reg2);
                break;
            }
        }

    case 6:                                     // OPQ rA, rB
        stringOperation = operation_Switch(fn); //( AND , SUB, XOR ... etc)
        register1 = r_Switch(reg1);
        register2 = r_Switch(reg2);

        if (reg2 >= 10 && reg2 < 16 && reg1 >= 10 && reg1 < 16)
        {
            letter = letter_Switch(reg1);
            letter2 = letter_Switch(reg2);
            ret = fprintf(out, "    %-8s %s, %s  # %d%d%s%s\n", stringOperation, register1, register2, code, 0, letter, letter2);
            break;
        }
        if (reg2 >= 10 && reg2 < 16)
        {
            letter = letter_Switch(reg2);
            ret = fprintf(out, "    %-8s %s, %s  # %d%d%d%s\n", stringOperation, register1, register2, code, 0, reg1, letter);
            break;
        }
        if (reg1 >= 10 && reg1 < 16)
        {
            letter = letter_Switch(reg1);
            ret = fprintf(out, "    %-8s %s, %s  # %d%d%s%d\n", stringOperation, register1, register2, code, 0, letter, reg2);
            break;
        }
        if (reg1 <= 10 && reg2 <= 10)
        {
            ret = fprintf(out, "    %-8s %s, %s  # %d%d%d%d\n", stringOperation, register1, register2, code, fn, reg1, reg2);
            break;
        }
        else
        {
            ret = fprintf(out, "    %-8s %s, %s  # %d%d%d%d\n", stringOperation, register1, register2, code, fn, reg1, reg2);
            break;
        }

    case 10: // A== pushQ
        register1 = r_Switch(reg1);
        ret = fprintf(out, "    %-8s %s  # %s%d%d%s\n", pushq, register1, "A", 0, reg1, "F");
        break;

    case 11: //B == popQ
        register1 = r_Switch(reg1);
        ret = fprintf(out, "    %-8s %s  # %s%d%d%s\n", popq, register1, "B", 0, reg1, "F");
    }
    return ret;
}
