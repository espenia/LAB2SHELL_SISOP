#ifndef UTILS_H
#define UTILS_H

#include "defs.h"

char* split_line(char* buf, char splitter);

int block_contains(char* buf, char c);


int do_fork(void);
char *shift_left(char *str, unsigned int shift);
#endif // UTILS_H
