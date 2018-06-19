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
#include <fcntl.h> // header for create the file
#include <sys/socket.h>
#include <netinet/in.h>
#include <netdb.h>
#include <strings.h>

#include <assert.h>
#include <zlib.h>

//#define SHELL 's'
#define PORT 'p'
#define LOG 'l'
#define COMPRESS 'c'

struct termios initial, termode;
const int BUFFER_SIZE = 256;
const int OUTBUFFER_SIZE = 5000;
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
	int portflag = 0, compressflag = 0;
	int portnum = 0;
	socklen_t clilen;
	int sockfd, newsockfd;
	z_stream strm1, strm2;
	int to_child_pipe[2];
	int to_parent_pipe[2];
	int ret;
	pid_t child_pid;

    struct sockaddr_in serv_addr, cli_addr;

	static struct option long_options[]=
	{
		{"port", 1, NULL, PORT},
		{"compress", 0, NULL, COMPRESS},
     	   	{0, 0, 0, 0}
	};
	signal(SIGPIPE, handler);
	int c;
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
        	case PORT:
        			portflag = 1;
        			portnum = atoi((char*) optarg); // port number
        			break;
        	
        	case COMPRESS:
        			compressflag = 1;
        			break;
        	default:
        			fprintf(stderr, "Error! Unrecognized argument: %s\n", argv[optind-1]);
        			fprintf(stderr, "Usage: ./lab1b-server --port=# [--compress]\n");
                    exit(1);
        }
	}

	if(portflag == 0)
	{
		fprintf(stderr, "Error! Switch <--port=port#> is require!\n");
        exit(1);
	}

	// CREATE A NEW SOCKET
	// socket (address domain, type of socket, protocol)
	sockfd = socket(AF_INET, SOCK_STREAM, 0);
	if(sockfd < 0)
	{
		fprintf(stderr, "Error on opening socket: %s\n", strerror(errno));
		exit(1);
	}

	// sets all values in a buffer to zero
	// bzero((char *) &serv_addr, sizeof(serv_addr));
	// bcopy((char*) server->h_addr, (char*) &serv_addr.sin_addr.s_addr, server->h_length);
	// from open source online, (also TA mentioned in class) memset() is preferred over bzero()
	// and memcpy() is preferred over the bcopy()
	memset((char*) &serv_addr, 0, sizeof(serv_addr));
	serv_addr.sin_family = AF_INET;
	serv_addr.sin_addr.s_addr = INADDR_ANY;
	serv_addr.sin_port = htons(portnum);

	// client establishs a connection to the server
	if(bind(sockfd, (struct sockaddr *)&serv_addr, sizeof(serv_addr)) < 0)
	{
		fprintf(stderr, "Error! Fail on binding!\n");
		exit(1);
	}
	if(listen(sockfd,5) < 0)
	{
		fprintf(stderr, "Error! Fail on listening!\n");
		exit(1);
	}
	clilen = sizeof(cli_addr);
	// accept the connection from the client
	newsockfd = accept(sockfd, (struct sockaddr*) &cli_addr, &clilen);
	if(newsockfd<0)
	{
		fprintf(stderr, "Error! Fail on accept!\n");
		exit(1);
	}

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
		char ch;
		ssize_t check3;
		unsigned char buffer[BUFFER_SIZE];
		unsigned char outputbuffer[OUTBUFFER_SIZE]; // output data buffer
		for(;;)
		{
			fds[0].fd = newsockfd; // describing the keyboard (stdin)
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
				exit(1);
			}
			
			// read input from the keyboard, echo it to stdout, and forward it to the shell
			if(fds[0].revents & POLLIN)
			{
				
				check3 = read(newsockfd, buffer, BUFFER_SIZE);
				if(check3 < 0)
				{
					fprintf(stderr, "Error on reading input from the keyboard: %s\n", strerror(errno));
					exit(1);
				}
				if(compressflag == 1)
				{
					int checkinf1 = -1;
					strm1.zalloc = Z_NULL;
					strm1.zfree = Z_NULL;
					strm1.opaque = Z_NULL;

					// initialize the zlib state for decompression
					checkinf1 = inflateInit(&strm1);
					if(checkinf1 != Z_OK)
					{
						inflateEnd(&strm1);
						fprintf(stderr, "inflateInit() failed!\n");
						exit(1);
					}
					
					strm1.avail_in = check3;
					strm1.next_in = buffer;
					strm1.avail_out = OUTBUFFER_SIZE;
					strm1.next_out = outputbuffer;

					do{
						inflate(&strm1, Z_SYNC_FLUSH);
					} while(strm1.avail_in > 0);
	
					inflateEnd(&strm1);
					ret = OUTBUFFER_SIZE - strm1.avail_out;

					for(int i=0; i<ret; i++)
					{
						ch = outputbuffer[i];
						if(outputbuffer[i] == '\r' || outputbuffer[i] == '\n') // map received <cr> or <lf> into <cr><lf> 
						{
							char temp = '\n';
							write(to_child_pipe[1], &temp, 1);
						}
						else if((outputbuffer[i] == 0x03) || (outputbuffer[i] == 0x04))
						{
							// close the related pipe to terminate
							close(to_child_pipe[1]);
							close(to_parent_pipe[0]);
							close(sockfd);
							close(newsockfd);

							int child_status;
							int end_ID;
							end_ID = waitpid(child_pid, &child_status, WNOHANG);

							if(end_ID == -1)
							{
								fprintf(stderr, "Error on waitpid(): %s\n", strerror(errno));
								exit(1);
							}

							// SHELL EXIT SIGNAL=# STATUS=#
							if(end_ID == child_pid)
							{
								close(to_parent_pipe[0]);
								fprintf(stderr, "SHELL EXIT SIGNAL=%d STATUS=%d\n", WTERMSIG(child_status), WEXITSTATUS(child_status));
								exit(0);	
							}
						}
						else
						{
							write(to_child_pipe[1], &ch, 1);
						}
					}
				}
				else // not compressed, no need to decompress
				{
					for(int i=0; i<check3; i++)
					{
						ch = buffer[i];
						if(buffer[i] == '\r' || buffer[i] == '\n') // map received <cr> or <lf> into <cr><lf> 
						{
							char temp = '\n';
							write(to_child_pipe[1], &temp, 1);
						}
						else if(buffer[i] == 0x03) // CTRLC
						{
							kill(child_pid, SIGINT);
				
						}
						else if(buffer[i] == 0x04) // CTRLD
						{
							close(to_child_pipe[1]);
							close(to_parent_pipe[0]);
                                                        close(sockfd);
                                                        close(newsockfd);
							int child_status;
                                                        int end_ID;
                                                        end_ID = waitpid(child_pid, &child_status, WNOHANG);

                                                        if(end_ID == -1)
                                                        {
                                                                fprintf(stderr, "Error on waitpid(): %s\n", strerror(errno));
                                                                exit(1);
                                                        }
							if(end_ID == child_pid)
                                                        {
                                                                close(to_parent_pipe[0]);
                                                                fprintf(stderr, "SHELL EXIT SIGNAL=%d STATUS=%d\n", WTERMSIG(child_status), WEXITSTATUS(child_status));
                                                                exit(0);
                                                        }
						}
						else
						{
							write(to_child_pipe[1], &ch, 1);
						}
					}
				}
			}


			/*The field revents is an output parameter, filled by the kernel with
			the events that actually occurred.  */

			if(fds[0].revents & (POLLHUP | POLLERR))
			{			
				close(to_child_pipe[1]);
				int child_status;
				int end_ID;
				end_ID = waitpid(child_pid, &child_status, WNOHANG);

				if(end_ID == -1)
				{
					fprintf(stderr, "Error on waitpid(): %s\n", strerror(errno));
					exit(1);
				}

				// SHELL EXIT SIGNAL=# STATUS=#
				if(end_ID == child_pid)
				{
					close(to_parent_pipe[0]);
					fprintf(stderr, "SHELL EXIT SIGNAL=%d STATUS=%d\n", WTERMSIG(child_status), WEXITSTATUS(child_status));
					exit(0);	
				}
			}

			// read input from the shell pipe  
			if(fds[1].revents & POLLIN)
			{
				check3 = read(to_parent_pipe[0], buffer, BUFFER_SIZE);
				if(check3 < 0)
				{
					fprintf(stderr, "Error on reading input from the shell: %s\n", strerror(errno));
					exit(1);
				}

				if(compressflag == 1)
				{
					int checkdef1 = -1;
					strm2.zalloc = Z_NULL;
					strm2.zfree = Z_NULL;
					strm2.opaque = Z_NULL;

					// initialize the zlib state for compression
					checkdef1 = deflateInit(&strm2, Z_DEFAULT_COMPRESSION);
					if(checkdef1 != Z_OK)
					{
						deflateEnd(&strm2);
						fprintf(stderr, "deflateInit() failed!\n");
						
						exit(1);
					}
					
					strm2.avail_in = check3;
					strm2.next_in = buffer;
					strm2.avail_out = OUTBUFFER_SIZE;
					strm2.next_out = outputbuffer;
					do{
						deflate(&strm2, Z_SYNC_FLUSH);
					} while(strm2.avail_in > 0);

					ret = OUTBUFFER_SIZE - strm2.avail_out;
					write(newsockfd, &outputbuffer, ret);
					deflateEnd(&strm2);
			
				}
				else
				{
					for(int i=0; i<check3; i++)
					{
						ch = buffer[i];
						write(newsockfd, &ch, 1);
					}
				}

				
			
			}

			if(fds[1].revents & (POLLHUP | POLLERR))
			{
				
				int child_status;
				int end_ID;
				end_ID = waitpid(child_pid, &child_status, WNOHANG);

				if(end_ID == -1)
				{
					fprintf(stderr, "Error on waitpid(): %s\n", strerror(errno));
					exit(1);
				}

				// SHELL EXIT SIGNAL=# STATUS=#
				if(end_ID == child_pid)
				{
					close(to_parent_pipe[0]);
					fprintf(stderr, "SHELL EXIT SIGNAL=%d STATUS=%d\n", WTERMSIG(child_status), WEXITSTATUS(child_status));
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
			exit(1);
		}
	}
	else // child_pid < 0
	{
		fprintf(stderr, "Ooh..fork() failed: %s\n", strerror(errno));
		exit(1);
	}
	exit(0);
}


