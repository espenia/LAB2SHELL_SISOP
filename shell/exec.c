#include "exec.h"
#include "utils.h"

// sets "key" with the key part of "arg"
// and null-terminates it
//
// Example:
//  - KEY=value
//  arg = ['K', 'E', 'Y', '=', 'v', 'a', 'l', 'u', 'e', '\0']
//  key = "KEY"
//
static void
get_environ_key(char* arg, char* key) {

	int i;
	for (i = 0; arg[i] != '='; i++)
		key[i] = arg[i];

	key[i] = END_STRING;
}

// sets "value" with the value part of "arg"
// and null-terminates it
// "idx" should be the index in "arg" where "=" char
// resides
//
// Example:
//  - KEY=value
//  arg = ['K', 'E', 'Y', '=', 'v', 'a', 'l', 'u', 'e', '\0']
//  value = "value"
//
static void
get_environ_value(char* arg, char* value, int idx) {

	size_t i, j;
	for (i = (idx + 1), j = 0; i < strlen(arg); i++, j++)
		value[j] = arg[i];

	value[j] = END_STRING;
}

// sets the environment variables received
// in the command line
//
// Hints:
// - use 'block_contains()' to
// 	get the index where the '=' is
// - 'get_environ_*()' can be useful here
static void
set_environ_vars(char** eargv, int eargc) {
    for (int i = 0; i < eargc; ++i) {
        int pos = 0;
        if ((pos = block_contains(eargv[i], '=')) == -1)
            continue;
        char *key = malloc(strlen(eargv[i])+1);
        char *value = malloc(strlen(eargv[i])+1);
        get_environ_key(eargv[i], key);

        get_environ_value(eargv[i], value, pos);
        int j = do_fork();
        if (j < 0)
            perror("error al forkear");
        if (j == 0) {
            if (setenv(key, value,0) < 0)
                perror("error al setear environ var");
            free(key);
            free(value);
        } else {

        }
    }

} 

// opens the file in which the stdin/stdout/stderr
// flow will be redirected, and returns
// the file descriptor
//
// Find out what permissions it needs.
// Does it have to be closed after the execve(2) call?
//
// Hints:
// - if O_CREAT is used, add S_IWUSR and S_IRUSR
// 	to make it a readable normal file
//no lo uso
static int
open_redir_fd(char* file, int flags) {
    int fd;
    if (flags == O_CREAT) {
        fd = open(file, O_WRONLY|flags, S_IWUSR|S_IRUSR);
        return fd;
    }
    else {
        fd = open(file, flags);
        return fd;
    }


}



static void close_fds(int fdin, int fdout, int fderr, struct execcmd *r) {
    if (strlen(r->in_file) > 0)
        if (close(fdin) < 0)
            perror("Error al cerrar fdin");
    if (strlen(r->out_file) > 0)
        if (close(fdout) < 0)
            perror("Error al cerrar fdout");
    if (strlen(r->err_file) > 0)
        if (close(fderr) < 0)
            perror("Error al cerrar fderr");
}

static int run_command(struct execcmd* cmd) {
    char **argv = malloc(sizeof(char*)*(cmd->argc+1));
    char cwd[BUFLEN];
    if (getcwd(cwd, sizeof(char)*BUFLEN) == NULL)
        perror("expand_environ_var: error al buscar current directory");
    argv[0] = cwd;
    for (int j = 1; j < cmd->argc+1; ++j) {
        argv[j] = cmd->argv[j];
    }
    if (execvpe(cmd->argv[0], argv, cmd->eargv) < 0) {
        free(argv);
        return -1;
    }
    free(argv);
    return 0;
}

static int run_command_redir(struct execcmd *r) {

    set_environ_vars(r->eargv, r->eargc);

    int fdin, fdout, fderr;
    if (strlen(r->in_file) > 0) {
        fdin = open_redir_fd(r->in_file, O_RDONLY);
        if (dup2(fdin, 0) < 0)
            perror("Se abrio el fd incorrecto se esperaba el 0 (stdin)");
    }
    if (strlen(r->out_file) > 0) {
        fdout = open_redir_fd(r->out_file, O_CREAT);
        if (dup2(fdout, 1) < 0)
            perror("Se abrio el fd incorrecto se esperaba el 1 (stdout)");
    }
    if (strlen(r->err_file) > 0) {
        fderr = open_redir_fd(r->err_file, O_CREAT);
        if (dup2(fderr, 2) < 0)
            perror("Se abrio el fd incorrecto se esperaba el 2 (stderr)");
    }


    char **argv = malloc(sizeof(char*)*(r->argc+1));
    char cwd[BUFLEN];
    if (getcwd(cwd, sizeof(char)*BUFLEN) == NULL)
        perror("expand_environ_var: error al buscar current directory");
    argv[0] = cwd;
    for (int j = 1; j < r->argc+1; ++j) {
        argv[j] = r->argv[j];
    }
    if (execvpe(r->argv[0], argv, r->eargv) < 0) {
        free(argv);
        return -1;
    }
    free(argv);

    close_fds(fdin, fdout, fderr, r);
    return 0;

}

static int run_pipe_commnad(struct pipecmd* p) {
    int fds[2] = {STDIN_FILENO, STDOUT_FILENO};
    if (pipe(fds) < 0) {
        perror("error al realizar pipe");
        return -1;
    }
    int i = do_fork();
    if (i < 0) {
        perror("error al realizar fork");
        return -1;
    }
    if (i != 0) {
        if (dup2(fds[1], STDOUT_FILENO) < 0) {
            perror("error al redireccionar stdout");
            return -1;
        }
        exec_cmd(p->leftcmd);
        if (close(fds[1]) < 0) {
            perror("error al cerrar write");
            return -1;
        }

    } else {

        if (dup2(fds[0], STDIN_FILENO) < 0) {
            perror("error al redireccionar stdin");
            return -1;
        }
        if (close(fds[1]) < 0) {
            perror("error al cerrar write");
            return -1;
        }
        exec_cmd(p->rightcmd);
    }
    if (close(fds[0]) < 0) {
        perror("error al cerrar read");
        return -1;
    }
    return 0;
}

// executes a command - does not return
//
// Hint:
// - check how the 'cmd' structs are defined
// 	in types.h
// - casting could be a good option
void
exec_cmd(struct cmd* cmd) {

	// To be used in the different cases
	struct execcmd* e;
	struct backcmd* b;
	struct execcmd* r;
	struct pipecmd* p;

	switch (cmd->type) {

        case EXEC: {
            // spawns a command
            //
            // Your code here

            e = (struct execcmd *) cmd;
            if (run_command(e) < 0) {
                perror("error al ejecutar comando 334");
                return;
            }

            break;
        }


        case BACK: {
            // runs a command in background
            //
            // Your code here

            b = (struct backcmd *) cmd;
            e = (struct execcmd *) b->c;
            if (run_command(e) < 0) {
                perror("error al ejecutar comando");
                return;
            }
            break;

        }

        case REDIR: {
            // changes the input/output/stderr flow
            //
            // To check if a redirection has to be performed
            // verify if file name's length (in the execcmd struct)
            // is greater than zero
            r = (struct execcmd *) cmd;

            if (run_command_redir(r) < 0) {
                perror("error al ejecutar comando");
                _exit(-1);

            }
            break;
        }

        case PIPE: {
            // pipes two commands
            //
            // Your code here
            p = (struct pipecmd *) cmd;
            if (!p->leftcmd || !p->rightcmd)
                perror("No se creo el pipe izq o el derecho");
            if (run_pipe_commnad(p) < 0) {
                perror("error al ejecutar comando");
                _exit(-1);
            }
            break;
        }
    }
}
