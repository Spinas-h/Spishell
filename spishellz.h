#ifndef SPISHELLZ_H
#define SPISHELLZ_H

void execute(char *input);
void exec_pipe(char *pre_pipe, char *post_pipe);
void exec_append_out(char *pre_operator, char *post_operator);
void exec_input_redir(char *pre_operator, char *post_operator);

#define MAX_HISTORY 1000
#define DIR_COLOR "\033[1;34m"
#define EXEC_COLOR "\033[1;36m"
#define CYAN    "\033[1;36m"
#define C_RESET   "\033[0m"
#define YELLOW "\033[93m"


#endif

