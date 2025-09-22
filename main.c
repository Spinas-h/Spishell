#include <stdio.h>
#include <stdlib.h>
#include <string.h>
#include <unistd.h>
#include "spishellz.h"
#include<sys/types.h>
#include<sys/wait.h>
#include <signal.h>
#include <termios.h>

char *history[MAX_HISTORY];
int history_count=0;
int history_index;
char buffer[1024];
int len=0;
char *cwd;
int cursor=0;
int redir_position;

void refresh_prompt_with(const char *buf) {
	printf("\33[2K\r");
	char *cwd = getcwd(NULL, 0);
	char *last = strrchr(cwd, '/');
	last = (last != NULL) ? last + 1 : cwd;
	printf(YELLOW "📘%s" CYAN "🐚spi➤" C_RESET, last);
	printf("%s", buf);
	fflush(stdout);
	free(cwd);
}

void prev_command()
{
	if(history_index>0)
	{
		history_index--;
		strcpy(buffer, history[history_index]);
		len=strlen(buffer);
		cursor=len;
		refresh_prompt_with(buffer);
		return;
	}
}

void next_command()
{
	if(history_index<history_count-1)
	{
		history_index++;
		strcpy(buffer, history[history_index]);
		len=strlen(buffer);
		cursor=len;
		refresh_prompt_with(buffer);
		return;
	}
	else if(history_index == history_count-1)
	{
		history_index = history_count;
		buffer[0] = '\0';
		len = 0;
		cursor = 0;
		refresh_prompt_with(buffer);
	}
}

char *read_input()
{
	struct termios old_settings;
	tcgetattr(STDIN_FILENO, &old_settings);

	struct termios new_settings=old_settings;
	new_settings.c_lflag &=~(ICANON | ECHO);
	tcsetattr(STDIN_FILENO, TCSANOW, &new_settings);
	buffer[0]='\0';
	len=0;
	cursor=0;

	char c;
	while(read(STDIN_FILENO, &c, 1)==1)
	{
		if(c==10)
		{
			buffer[len] = '\0';
			break;
		}
		if(c==127 || c==8)
		{
			if(cursor > 0)
			{
				memmove(&buffer[cursor-1], &buffer[cursor], len - cursor);
				cursor--;
				len--;
				buffer[len] = '\0';

				printf("\33[2K\r");
				char *cwd = getcwd(NULL, 0);
				char *last = strrchr(cwd, '/');
				last = (last != NULL) ? last + 1 : cwd;
				printf(YELLOW "📘%s" CYAN "🐚spi➤" C_RESET, last);
				printf("%s", buffer);

				for(int i = len; i > cursor; i--)
					printf("\b");
				fflush(stdout);
				free(cwd);
			}
		}
		else if(c==27)
		{
			char nex[2];
			if(read(STDIN_FILENO, &nex[0], 1)==1 && read(STDIN_FILENO, &nex[1], 1)==1)
			{
				if(nex[0]=='[')
				{
					if(nex[1]=='A')
						prev_command();
					else if(nex[1]=='B')
						next_command();
					else if(nex[1]=='D' && cursor>0)
					{
						cursor--;
						printf("\b");
						fflush(stdout);
					}
					else if(nex[1]=='C' && cursor<len)
					{
						printf("%c", buffer[cursor]);
						cursor++;
						fflush(stdout);
					}
				}
			}
		}
		else if(c >= 32 && c <= 126)
		{
			if(len<sizeof(buffer)-1)
			{
				memmove(&buffer[cursor + 1], &buffer[cursor], len - cursor);
				buffer[cursor] = c;
				len++;
				cursor++;
				buffer[len] = '\0';

				printf("\33[s");
				printf("%s", &buffer[cursor - 1]);
				printf("\33[u");
				printf("%c", c);
				fflush(stdout);
			}
		}
	}

	tcsetattr(STDIN_FILENO, TCSANOW, &old_settings);

	return strdup(buffer);
}

void load_history()
{
	char path[1024];
	snprintf(path, sizeof(path), "%s/.spishell_history", getenv("HOME"));
	FILE *file=fopen(path, "r");
	if(!file)
		return;

	char line[1024];

	while(fgets(line, sizeof(line), file) != NULL && history_count < MAX_HISTORY)
	{
		line[strcspn(line, "\n")]='\0';
		if(strlen(line) > 0)
		{
			history[history_count++]=strdup(line);
		}
	}
	fclose(file);
}

void save_history(const char *cmd)
{
	if(cmd[0]=='\0') return;

	if(history_count<MAX_HISTORY)
	{
		history[history_count++]=strdup(cmd);
	}
	else
	{
		free(history[0]);
		for(int i = 0; i < MAX_HISTORY-1; i++)
		{
			history[i] = history[i+1];
		}
		history[MAX_HISTORY-1] = strdup(cmd);
	}

	char path[1024];
	snprintf(path, sizeof(path), "%s/.spishell_history", getenv("HOME"));
	FILE *file=fopen(path, "a");
	if(file)
	{
		fprintf(file, "%s\n", cmd);
		fclose(file);
	}
}

void print_history()
{
	for(int i=0; i<history_count; i++)
	{
		printf("%d %s\n",i+1, history[i]);
	}
}

int main()
{
	load_history();
	history_index=history_count;

	while(1)
	{
		cwd=getcwd(NULL, 0);
		char *last = strrchr(cwd, '/');
		last = (last != NULL) ? last + 1 : cwd;
		printf( YELLOW "📘%s" CYAN "🐚spi➤" C_RESET, last);
		fflush(stdout);

		char *input=read_input();
		printf("\n");

		if(!input)
		{
			printf("\n");
			break;
		}

		history_index=history_count;


		if(strcmp(input,"exit")==0)
		{
			free(input);
			break;
		}

		if(strcmp(input, "history")==0)
		{
			print_history();
			free(input);
			continue;
		}

		if(strlen(input) > 0 && (history_count==0 || strcmp(history[history_count-1], input)!=0))
			save_history(input);

		char pre_input[500], post_input[500];
		char *red1 = strchr(input, '>');
		char *red2 = strstr(input, ">>");
		char *red3 = strchr(input, '<');

		if(red2!=NULL)
		{
			redir_position=red2 - input;
			strncpy(pre_input, input, redir_position);
			pre_input[redir_position]='\0';
			strcpy(post_input, red2+2);
			while(post_input[0]==' ')
				memmove(post_input, post_input+1, strlen(post_input));

			exec_append_out(pre_input, post_input);
		}
		else if(red3!=NULL)
		{
			redir_position=red3-input;
			strncpy(pre_input, input, redir_position);
			pre_input[redir_position]='\0';
			strcpy(post_input, red3+1);
			while(post_input[0]==' ')
				memmove(post_input, post_input+1, strlen(post_input));

			exec_input_redir(pre_input, post_input);
		}
		else if(red1!=NULL)
		{
			redir_position=red1-input;

		}

		if(strchr(input, '|')==NULL)
		{
			execute(input);
		}
		else
		{
			char *pipe_pos = strchr(input, '|');
			if(pipe_pos)
			{
				int pipe_index = pipe_pos - input;
				strncpy(pre_input, input, pipe_index);
				pre_input[pipe_index] = '\0';

				strcpy(post_input, pipe_pos + 1);
				while(post_input[0]==' ')
					memmove(post_input, post_input+1, strlen(post_input));

				exec_pipe(pre_input, post_input);
			}
		}

		free(input);
		free(cwd);
	}

	for(int i = 0; i < history_count; i++)
	{
		free(history[i]);
	}

	return 0;
}
