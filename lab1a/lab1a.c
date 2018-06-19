// NAME: Yanyin Liu
// EMAIL: yanyinliu8@gmail.com
// ID: 604952257


#include <stdio.h>
#include <stdlib.h>
#include <termios.h> // for termios functions
#include <unistd.h> 
#include <errno.h>
#include <string.h>
#include <getopt.h> // header file for getopt_long()
#include <signal.h> // header file for signal()
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




	//  gets the parameters associated with the object referred by 
	//  fd and stores them in the termios structure referenced by termios_p. 
	tcgetattr(0, &initial);
	termode.c_iflag = ISTRIP; /* only lower 7 bits	*/
	termode.c_oflag = 0; /* no processing	*/
	termode.c_lflag = 0; /* no processing	*/

	/* sets the parameters associated with the terminal */
	tcsetattr(0, TCSANOW, &termode);

	if(isShell == 1)
	{
		signal(SIGPIPE, handler);
		int to_child_pipe[2];
		int to_parent_pipe[2];
		// pid_t child_pid = -1;

		// pipe before fork()
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

		if(child_pid > 0) // parent process
		{
			close(to_parent_pipe[1]);
			close(to_child_pipe[0]);

			struct pollfd fds[2];
			

			int c;
			ssize_t check3;
			char buffer[BUFFER_SIZE];
			for(;;)
			{
				fds[0].fd = 0; // describing the keyboard (stdin)
				fds[1].fd = to_parent_pipe[0]; // describing the pipe that returns output from the shell (read)
				fds[0].events = POLLIN | POLLHUP | POLLERR;
				fds[1].events = POLLIN | POLLHUP | POLLERR;
				fds[0].revents = 0;
				fds[1].revents = 0;

				c = poll(fds, 2, 0);
				if (c==0) // timeout
				{
					continue;
				}
				if(c < 0)
				{
					fprintf(stderr, "Ooh.. poll() failed: %s\n", strerror(errno));
					// tcsetattr(0, TCSANOW, &initial);
					exit(1);
				}
				
				// read input from the keyboard, echo it to stdout, and forward it to the shell
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
						// if(check3 == 0)
						// {
						// 	continue; //EOF
						// }
						if(buffer[i] == '\r' || buffer[i] == '\n') // map received <cr> or <lf> into <cr><lf> 
						{
							write(1, "\r\n", 2);
							// write(to_child_pipe[1], '\n', 1);
							char temp = '\n';
							write(to_child_pipe[1], &temp, 1);
						}
						else if(buffer[i] == 0x03) // CTRLC
						{
							kill(child_pid, SIGINT);
						}
						else if(buffer[i] == 0x04) // CTRLD
						{
							//Upon receiving an EOF (^D, or 0x04) from the terminal, close the pipe to the shell
							//but continue processing input from the shell

							// close pipe fd, after process is through writing to it (e.g., because you received an ^D)
							close(to_child_pipe[1]);
							
						}
						else
						{
							write(1, buffer+i, 1);
							write(to_child_pipe[1], buffer+i, 1);
						}
					}
				}

				/*The field revents is an output parameter, filled by the kernel with
				the events that actually occurred.  */
				if(fds[0].revents & (POLLHUP | POLLERR))
				{
					fprintf(stderr, "Error on keyboard: %s\n", strerror(errno));
					tcsetattr(0, TCSANOW, &initial);
					exit(0);				
				}


				// read input from the shell pipe and write it to stdout. 
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
						if(buffer[i] == '\n' || buffer[i] == '\r') // received an <lf> from shell, print it to the screen as <cr><lf>
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
					//collect the shell's exit status 
					int child_status;
					int end_ID;
					// end_ID = wait(child_pid, &child_status, WNOHANG | WUNTRACED | WCONTINUED);

					end_ID = waitpid(child_pid, &child_status, 0);
					if(end_ID == -1)
					{
						fprintf(stderr, "Error on waitpid(): %s\n", strerror(errno));
						tcsetattr(0, TCSANOW, &initial);
						exit(1);
					}
					// SHELL EXIT SIGNAL=# STATUS=#
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
		else if(child_pid == 0) // child process
		{
			close(to_child_pipe[1]);
			close(to_parent_pipe[0]); // we read from the terminal, this end is not needed
			dup2(to_child_pipe[0], 0); //standard input is a pipe from the terminal process,
			dup2(to_parent_pipe[1], 1); // standard output is a pipe to the terminal process
			dup2(to_parent_pipe[1], 2); //standard error is a pipe to the terminal process
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
		// read input from the keyboard into a buffer
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
					continue; //EOF
				}
				if(buffer[i] == '\r' || buffer[i] == '\n') // map received <cr> or <lf> into <cr><lf> 
				{
					check2 = write(1, "\r\n", 2);
				}
				else if(buffer[i] == 0x04) // CTRLD
				{
					// restore (reset) normal terminal modes and exit
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


	// restore (reset) normal terminal modes and exit
	tcsetattr(0, TCSANOW, &initial);
	exit(0);
}


