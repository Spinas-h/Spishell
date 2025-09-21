#include <stdio.h>
#include <string.h>
#include <stdlib.h>
#include <sys/wait.h>
#include <unistd.h>
#include <sys/types.h>
#include <dirent.h>
#include <sys/stat.h>
#include <fcntl.h>
#include <termios.h>

void parse_pipe_command(char *, char**);
void parent_process(pid_t pid);
void exec_builtin(char **, int);
void sls(char **);
void exec_ai(char **);


void execute(char *input)
{
	
	char *token[100];
	int i=0, bg=0, flag=-1;
	token[i]=strtok(input, " ");
	 if(strcmp(token[i], "ls")==0)
	{
		token[++i]="--color";
	}
	else if(strcmp(token[i], "spi")==0)
	{
		while(token[i]!=NULL)
		{
			token[++i]=strtok(NULL, " ");
		}
		token[i]=NULL;
		exec_ai(token);
		
	}
		
	while(token[i]!=NULL)
	{
		i++;
		token[i]=strtok(NULL, " "); 
		if(token[i]!=NULL && strcmp(token[i],"&")==0)
		{
			token[i]=NULL;
			bg=1;
		}		
	}
	token[i]=NULL;
	
	if(strcmp(token[0], "cd")==0 )
	{
		flag=0;
		exec_builtin(token, flag);
		return;
	}
	
	
		
	pid_t pid=fork();
	if(pid<0)
	{
		perror("fork failed");
	}
	
	else if(pid==0)
	{
		execvp(token[0], token);
	}
																								
	else
	{ 
		
		if(bg==1)
		{
			
			waitpid(pid, NULL, WNOHANG); 
			
		}
		else
		{
			wait(NULL);
		}
		

	}
			
}

void exec_pipe(char *pre_pipe, char *post_pipe)
{
	int fd[2]; 
	pipe(fd);
	pid_t pid=fork();
	if(pid<0)
	{
		perror("fork failed");
	}
	else if(pid==0)
	{
		close(fd[0]);
		dup2(fd[1], STDOUT_FILENO);
		close(fd[1]);
		char *token[100];
		parse_pipe_command(pre_pipe, token);
		execvp(token[0], token);
		perror("pipe's LHS not executed");
		exit(1);
		
		
	}
	
	
	pid_t pid1=fork();
	if(pid1<0)
		perror("fork failed");
	else if(pid1==0)
	{
		close(fd[1]);
		dup2(fd[0], STDIN_FILENO);
		close(fd[0]);
		char *token[100];
		parse_pipe_command(post_pipe, token);
		execvp(token[0], token);
		perror("RHS not executed");
		exit(1);	
	}
	close(fd[0]);
	close(fd[1]);
	parent_process(pid);
	parent_process(pid1);		
}

void parse_pipe_command(char *command, char **token)
{

	token[0]=strtok(command, " ");
	int i=0;
	while(token[i]!=NULL)
		token[++i]=strtok(NULL, " ");	
	token[i]=NULL;	
}

void parent_process( pid_t pid)
{
	{

		waitpid(pid, NULL, 0);
		
	} 
}

void exec_builtin(char **token, int cmd)
{
	   cmd++;
	   switch(cmd)
	   {
	   	case 1:
	   	char *path=token[1];
	   	if(chdir(path)!=0)
	   	{
	   		perror("cmd");
	   	}
	   	break;
	   	case2:
	   	printf("will come out in future ig");
	   	break;
	   	default:
	   	break;
	   }
}

void sls(char **token)
{
	printf("custom 'ls' command; not yet written yet);
}
/*👇calling ai won't work unless you have llama.cpp with deepseek 13B model installed in your device */
/*void exec_ai(char **input)
{
	struct termios orig_termios;
	tcgetattr(STDIN_FILENO, &orig_termios);
	char prompt[1024]="";
	int i=1;
	pid_t pid=fork();
	if(pid<0)
	{
		perror("failed forking");
	}
	else if(pid==0)
	{

		while(input[i]!=NULL)
		{
			strcat(prompt, input[i]);

			strcat(prompt, " ");

			i++;
		}
		prompt[strlen(prompt) - 1]='\0';
		int len=strlen(prompt);
		char command[1500];
		snprintf(command, sizeof(command),   "./llama.cpp/build/bin/llama-cli "
	    "-m ./llama.cpp/models/deepseek/deepseek-coder-1.3b-instruct-Q4_K_M.gguf " 
	    "-p \"Answer briefly: %s\" 2>/dev/null", prompt);


		FILE *fp=popen(command, "r");
		if(fp==NULL)
		{
			perror("failed to call the assistant");
			return;
		}
		char buffer[500];
		while(fgets(buffer, sizeof(buffer), fp)!=NULL)
		{
			printf("%s", buffer);
		}
		pclose(fp);
		tcsetattr(STDIN_FILENO, TCSANOW, &orig_termios);
	}


}*/

void exec_append_out(char *pre_operator, char *post_operator)
{
	char *file=strtok(post_operator, " ");
	int fd=open(file, O_WRONLY | O_CREAT | O_APPEND, 0644);
	if(fd<0)
	{
		perror("failed opening file");
		exit(1);
	}
	pid_t pid=fork();
	if(pid<0)
	{
		perror("fork failed");
	}
	else if (pid==0)
	{
		dup2(fd, STDOUT_FILENO);
		close(fd);
		char *token[100];
		parse_pipe_command(pre_operator, token);
		execvp(token[0], token);
		perror("failed");
		exit(1);
	}
	else
	{
		close(fd);
		waitpid(pid, NULL, 0);
	}			
}

void exec_input_redir(char *pre_operator, char *post_operator)
{
	char *file=strtok(post_operator, " ");
	int fd=open(file, O_RDONLY);
	if(fd<0)	
	{
		perror("failed opening the file");
		exit(1);
	}
	pid_t pid=fork();
	if(pid<0)
	{
		perror("fork failed");
	}
	else if(pid==0)
	{
		dup2(fd, STDIN_FILENO);
		close(fd);
		char *token[50];
		parse_pipe_command(pre_operator, token);
		execvp(token[0], token);
		perror("failed");
		exit(1);
	}
	else
	{
		close(fd);
		waitpid(pid, NULL, 0);
	}
}









