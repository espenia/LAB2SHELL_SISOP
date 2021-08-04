#include "utils.h"

// splits a string line in two
// acording to the splitter character
char*
split_line(char* buf, char splitter) {

	int i = 0;

	while (buf[i] != splitter &&
		buf[i] != END_STRING)
		i++;

	buf[i++] = END_STRING;

	while (buf[i] == SPACE)
		i++;

	return &buf[i];
}

// looks in a block for the 'c' character
// and returns the index in which it is, or -1
// in other case
int
block_contains(char* buf, char c) {

	for (size_t i = 0; i < strlen(buf); i++)
		if (buf[i] == c)
			return (int)i;

	return -1;
}


//utils by Esteban

int do_fork() {
    int i = fork();
    if (i < 0) {
        printf("Error en fork %d\n", i);
        return -1;
    }
    return i;
}

char *shift_left(char *str, unsigned int shift) {
    if (shift > strlen(str)) {
        perror("shift_left invalid shift amount");
    }
    for (int i = 0; i < (int)strlen(str) + 1 - shift; ++i)
        str[i] = str[i + shift];
    return str;
}

