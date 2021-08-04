#include "builtin.h"
#include "utils.h"
// returns true if the 'exit' call
// should be performed
//
// (It must not be called from here)
int
exit_shell(char* cmd) {
    if (strcmp(cmd, "exit") == 0)
        exit(1);
    return 0;
}

// returns true if "chdir" was performed
//  this means that if 'cmd' contains:
// 	1. $ cd directory (change to 'directory')
// 	2. $ cd (change to $HOME)
//  it has to be executed and then return true
//
//  Remember to update the 'prompt' with the
//  	new directory.
//
// Examples:
//  1. cmd = ['c','d', ' ', '/', 'b', 'i', 'n', '\0']
//  2. cmd = ['c','d', '\0']
int
cd(char* cmd) {
    char *temp = calloc(strlen(cmd)+1, sizeof(char*));
    char *buf = strcpy(temp, cmd);
    char *right = split_line(buf, ' ');
    if (strcmp(buf,"cd") != 0) {
        free(temp);
        return 0;
    }
    if (strcmp(right, "$HOME") == 0)
        right = strcpy(right, getenv("HOME"));
    if (chdir(right) < 0) {
        free(temp);
        perror("error al cambiar directorio");
        return 0;
    }
    free(temp);
	return -1;
}

// returns true if 'pwd' was invoked
// in the command line
//
// (It has to be executed here and then
// 	return true)
int
pwd(char* cmd) {

    if (strcmp(cmd,"pwd") != 0)
        return 0;
    char *cwd = getcwd(cmd, BUFLEN);
    printf("%s\n", cwd);
	// Your code here

	return -1;
}

