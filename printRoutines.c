#include <stdio.h>
#include <unistd.h>
#include "printRoutines.h"

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
