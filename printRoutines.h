

#ifndef _PRINTROUTINES_H_
#define _PRINTROUTINES_H_

#include <stdio.h>
void printZeroByte(FILE *out, int val);
void print_byte(FILE *out, int val);
void print_pos(FILE *out, int pos);
void print_halt(FILE *out);
void print_position(FILE *out, int pos);
void print_nop(FILE *out);
void print_quad(FILE *out, int val);
int printTwoBytes(FILE *out, int code, int fn, int reg1, int reg2);
#endif
