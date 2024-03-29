#include "runcmd.h"

int status = 0;
struct cmd* parsed_pipe;

// runs the command in 'cmd'
int
run_cmd(char* cmd) {

	pid_t p;
	struct cmd *parsed;

	// if the "enter" key is pressed
	// just print the promt again
	if (cmd[0] == END_STRING)
		return 0;

	// "cd" built-in call
	if (cd(cmd)) {
	    status = 0;
        return 0;
	}


	// "exit" built-in call
	if (exit_shell(cmd)) {
	    status = 0;
        return EXIT_SHELL;
	}


	// "pwd" buil-in call
	if (pwd(cmd)) {
	    status = 0;
        return 0;
	}


	// parses the command line
	parsed = parse_line(cmd);

	// forks and run the command
	if ((p = fork()) == 0) {

		// keep a reference
		// to the parsed pipe cmd
		// so it can be freed later
		if (parsed->type == PIPE)
			parsed_pipe = parsed;

		exec_cmd(parsed);
	}

	// store the pid of the process
	parsed->pid = p;

	// background process special treatment
	// Hint:
	// - check if the process is 
	// 	going to be run in the 'back'
	// - print info about it with esteban
	// 	'print_back_info()'
	//
	// Your code here
	// waits for the process to finish
	if (parsed->type != BACK) {
        waitpid(p, &status, 0);
	}
	else
	    print_back_info(parsed);
    print_status_info(parsed);
	free_command(parsed);

	return 0;
}

