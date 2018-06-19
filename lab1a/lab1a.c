// NAME: Yanyin Liu


#include <stdio.h>
#include <stdlib.h>
#include <termios.h> 
#include <unistd.h> 
#include <errno.h>
#include <string.h>
#include <getopt.h> 
#include <signal.h> 
#include <sys/stat.h>
#include <sys/types.h>
#include <poll.h>

#include <sys/wait.h>

struct termios initial, termode;
const int BUFFER_SIZE = 256;
#define SHELL 's'
pid_t child_pid;

void handler(int signo)
{
	if(signo == SIGPIPE)
    {
        kill(child_pid, SIGINT);
        tcsetattr(0, TCSANOW, &initial);
		exit(0);

    }
}

int main(int argc, char **argv)
{

	static struct option long_options[]=
	{
		{"shell", 0, NULL, SHELL},
        {0, 0, 0, 0}
	};

	int c;
	int isShell=0;
	while (1)
	{
		c = getopt_long(argc, argv, "", long_options, NULL);
		if(c == -1)
       	{
       		break;
        }
        fprintf(stderr, "c: %c, optind: %d, optarg: %s\n", c, optind, optarg);
        switch(c)
        {
        	case SHELL:
        			isShell = 1;
        			break;
        	default:
        			fprintf(stderr, "Error! Unrecognized argument: %s\n", argv[optind-1]);
                    exit(1);
        }
	}



	tcgetattr(0, &initial);
	termode.c_iflag = ISTRIP; 
	termode.c_oflag = 0;
	termode.c_lflag = 0; 

	tcsetattr(0, TCSANOW, &termode);

	if(isShell == 1)
	{
		signal(SIGPIPE, handler);
		int to_child_pipe[2];
		int to_parent_pipe[2];
		
		if(pipe(to_child_pipe) == -1)
		{
			fprintf(stderr, "Ooh..pipe() failed: %s\n", strerror(errno));
			exit(1);
		}
		if(pipe(to_parent_pipe) == -1)
		{
			fprintf(stderr, "Ooh..pipe() failed: %s\n", strerror(errno));
			exit(1);
		}


		child_pid = fork();

		if(child_pid > 0) 
		{
			close(to_parent_pipe[1]);
			close(to_child_pipe[0]);

			struct pollfd fds[2];
			

			int c;
			ssize_t check3;
			char buffer[BUFFER_SIZE];
			for(;;)
			{
				fds[0].fd = 0; 
				fds[1].fd = to_parent_pipe[0]; 
				fds[0].events = POLLIN | POLLHUP | POLLERR;
				fds[1].events = POLLIN | POLLHUP | POLLERR;
				fds[0].revents = 0;
				fds[1].revents = 0;

				c = poll(fds, 2, 0);
				if (c==0) 
				{
					continue;
				}
				if(c < 0)
				{
					fprintf(stderr, "Ooh.. poll() failed: %s\n", strerror(errno));
					
					exit(1);
				}
				
				
				if(fds[0].revents & POLLIN)
				{
					check3 = read(0, buffer, BUFFER_SIZE);
					if(check3 < 0)
					{
						fprintf(stderr, "Error on reading input from the keyboard: %s\n", strerror(errno));
						tcsetattr(0, TCSANOW, &initial);
						exit(1);
					}
					for(int i=0; i<check3; i++)
					{
						
						if(buffer[i] == '\r' || buffer[i] == '\n')  
						{
							write(1, "\r\n", 2);
							
							char temp = '\n';
							write(to_child_pipe[1], &temp, 1);
						}
						else if(buffer[i] == 0x03)
						{
							kill(child_pid, SIGINT);
						}
						else if(buffer[i] == 0x04) 
						{
							
							close(to_child_pipe[1]);
							
						}
						else
						{
							write(1, buffer+i, 1);
							write(to_child_pipe[1], buffer+i, 1);
						}
					}
				}

				if(fds[0].revents & (POLLHUP | POLLERR))
				{
					fprintf(stderr, "Error on keyboard: %s\n", strerror(errno));
					tcsetattr(0, TCSANOW, &initial);
					exit(0);				
				}


				if(fds[1].revents & POLLIN)
				{
					check3 = read(to_parent_pipe[0], buffer, BUFFER_SIZE);
					if(check3 < 0)
					{
						fprintf(stderr, "Error on reading input from the shell: %s\n", strerror(errno));
						tcsetattr(0, TCSANOW, &initial);
						exit(1);
					}
					for(int i=0; i<check3; i++)
					{
						if(buffer[i] == '\n' || buffer[i] == '\r') 
						{
							write(1, "\r\n", 2);
						}
						else
						{
							write(1, buffer+i, 1);
						}
					}
				
				}
				if(fds[1].revents & (POLLHUP | POLLERR))
				{
					close(to_child_pipe[1]);
					
					int child_status;
					int end_ID;
					
					end_ID = waitpid(child_pid, &child_status, 0);
					if(end_ID == -1)
					{
						fprintf(stderr, "Error on waitpid(): %s\n", strerror(errno));
						tcsetattr(0, TCSANOW, &initial);
						exit(1);
					}
					
					if(end_ID == child_pid)
					{	
						close(to_parent_pipe[0]);
						fprintf(stderr, "SHELL EXIT SIGNAL=%d STATUS=%d\r\n", WTERMSIG(child_status), WEXITSTATUS(child_status));
						tcsetattr(0, TCSANOW, &initial);
						exit(0);	
					}			
				}

			}

		}
		else if(child_pid == 0) 
		{
			close(to_child_pipe[1]);
			close(to_parent_pipe[0]); 
			dup2(to_child_pipe[0], 0); 
			dup2(to_parent_pipe[1], 1); 
			dup2(to_parent_pipe[1], 2);
			close(to_child_pipe[0]);
			close(to_parent_pipe[1]);

			char *execvp_argv[2];
			char execvp_filename[] = "/bin/bash";
			execvp_argv[0] = execvp_filename;
			execvp_argv[1] = NULL;

			if(execvp(execvp_filename, execvp_argv) == -1)
			{
				fprintf(stderr, "Ooh..execvp() failed: %s\n", strerror(errno));
				tcsetattr(0, TCSANOW, &initial);
				exit(1);
			}
		}
		else
		{
			fprintf(stderr, "Ooh..fork() failed: %s\n", strerror(errno));
			tcsetattr(0, TCSANOW, &initial);
			exit(1);
		}

	}
	else
	{
		ssize_t check1, check2;
		char buffer[BUFFER_SIZE];
		while(1)
		{
			check1 = read(0, buffer, BUFFER_SIZE);
			if(check1 < 0)
			{
				fprintf(stderr, "Error on reading input from the keyboard into a buffer: %s\n", strerror(errno));
				exit(1);
			}
			for(int i=0; i<check1; i++)
			{
				if(check1 == 0)
				{
					continue; 
				}
				if(buffer[i] == '\r' || buffer[i] == '\n') 
				{
					check2 = write(1, "\r\n", 2);
				}
				else if(buffer[i] == 0x04)
				{
					
					tcsetattr(0, TCSANOW, &initial);
					exit(0);
					
				}
				else
				{
					check2 = write(1, buffer+i, 1);
				}
				if(check2 < 0)
				{
					fprintf(stderr, "Error on writing the received characters back out to the display: %s\n", strerror(errno));
					exit(1);
				}
			}
		
		}
	}


	tcsetattr(0, TCSANOW, &initial);
	exit(0);
}


