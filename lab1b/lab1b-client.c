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
#include <fcntl.h> 
#include <sys/socket.h>
#include <netinet/in.h>
#include <netdb.h>
#include <strings.h>

#include <assert.h>
#include <zlib.h>


#define PORT 'p'
#define LOG 'l'
#define COMPRESS 'c'

struct termios initial, termode;
const int BUFFER_SIZE = 256;
const int OUTBUFFER_SIZE = 5000;


int main(int argc, char **argv)
{
	int portflag = 0, logflag = 0, compressflag = 0;
	int portnum = 0;
	char *logfile;
	int sockfd;
	z_stream strm1, strm2;

    struct sockaddr_in serv_addr;
    struct hostent *server;

	static struct option long_options[]=
	{
		{"port", 1, NULL, PORT},
		{"log", 1, NULL, LOG},
		{"compress", 0, NULL, COMPRESS},
        {0, 0, 0, 0}
	};

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
        			portnum = atoi((char*) optarg); 
        			break;
        	case LOG:
        			logflag = 1;
        			logfile = (char*) optarg; 
        			break;
        	case COMPRESS:
        			compressflag = 1;
        			break;
        	default:
        			fprintf(stderr, "Error! Unrecognized argument: %s\n", argv[optind-1]);
        			fprintf(stderr, "Usage: ./lab1b-client --port=# [--log=logfile] [--compress]\n");
                    exit(1);
        }
	}
	if(portflag == 0)
	{
		fprintf(stderr, "Error! Switch <--port=port#> is require!\n");
        exit(1);
	}

	int logfd = -1;
	if(logflag)
	{
		logfd=open(logfile, O_CREAT|O_WRONLY|O_TRUNC, 0666);
               
		if(logfd < 0)
		{
			fprintf(stderr, "Error! Unable to create and open the log file: %s\n", strerror(errno));
			exit(1);
		}
	}

	sockfd = socket(AF_INET, SOCK_STREAM, 0);
	if(sockfd < 0)
	{
		fprintf(stderr, "Error on opening socket: %s\n", strerror(errno));
		exit(1);
	}

	server = gethostbyname("localhost");
	if(server == NULL)
	{
		fprintf(stderr, "Error! The system could not locate the host: %s\n", "localhost");
		exit(1);
	}

	memset((char*) &serv_addr, 0, sizeof(serv_addr));
	serv_addr.sin_family = AF_INET;
	
	memcpy((char*) &serv_addr.sin_addr.s_addr, (char*) server->h_addr, server->h_length);
	serv_addr.sin_port = htons(portnum);

	if(connect(sockfd, (struct sockaddr*)&serv_addr, sizeof(serv_addr)) < 0)
	{
		fprintf(stderr, "Error! Fail to establish a connection to server!\n");
		exit(1);
	}

	tcgetattr(0, &initial);
	termode.c_iflag = ISTRIP; 
	termode.c_oflag = 0;
	termode.c_lflag = 0; 
	tcsetattr(0, TCSANOW, &termode);



	struct pollfd fds[2];
	int p, ret;
	int check3;
	char ch;

	
	for(;;)
	{
		fds[0].fd = 0; 
		fds[1].fd = sockfd;  
		fds[0].events = POLLIN | POLLHUP | POLLERR;
		fds[1].events = POLLIN | POLLHUP | POLLERR;
		fds[0].revents = 0;
		fds[1].revents = 0;

		p = poll(fds, 2, 0);
		if (p==0) 
		{
			continue;
		}
		if(p < 0)
		{
			fprintf(stderr, "Ooh.. poll() failed: %s\n", strerror(errno));
			tcsetattr(0, TCSANOW, &initial);
			exit(1);
		}
		
		unsigned char buffer[BUFFER_SIZE];
		unsigned char outputbuffer[OUTBUFFER_SIZE]; 
		if(fds[0].revents & POLLIN)
		{
			check3 = read(0, buffer, BUFFER_SIZE);
			if(check3 < 0)
			{
				fprintf(stderr, "Error on reading input from the keyboard: %s\n", strerror(errno));
				tcsetattr(0, TCSANOW, &initial);
				exit(1);
			}

			if(compressflag == 1)
			{		
				int checkdef1 = -1, checkdef2 = -1;
				strm1.zalloc = Z_NULL;
				strm1.zfree = Z_NULL;
				strm1.opaque = Z_NULL;
				checkdef1 = deflateInit(&strm1, Z_DEFAULT_COMPRESSION);
				if(checkdef1 != Z_OK)
				{
					deflateEnd(&strm1);
					fprintf(stderr, "deflateInit() failed!\n");
					tcsetattr(0, TCSANOW, &initial);
					exit(1);
				}
				
				strm1.avail_in = check3;
				strm1.next_in = buffer;
				strm1.avail_out = OUTBUFFER_SIZE;
				strm1.next_out = outputbuffer;

				do{
					checkdef2=deflate(&strm1, Z_SYNC_FLUSH);
					if(checkdef2 != Z_OK)
					{
						fprintf(stderr, "deflate() failed!\n");
						tcsetattr(0, TCSANOW, &initial);
						exit(1);
					}
				} while(strm1.avail_in > 0);

				ret = OUTBUFFER_SIZE-strm1.avail_out;

				if(logflag==1) 
				{
   	 				char s[256];
	    				sprintf(s, "SENT %d bytes: %s\n", ret, outputbuffer);
					write(logfd, s, strlen(s));
				}
				write(sockfd, &outputbuffer, ret);
				deflateEnd(&strm1);
			}

	
			if(logflag==1 && (!compressflag))
			{
   	 			char s[256];
    				sprintf(s, "SENT %d bytes: %s\n", check3, buffer);
				write(logfd, s, strlen(s));

			}

			for(int i=0; i<check3; i++)
			{
				ch = buffer[i];

				if(buffer[i] == '\r' || buffer[i] == '\n') 
				{
					
					write(1, "\r\n", 2);
					if(!compressflag)
					{
						write(sockfd, &ch, 1);
					}
				}
				else
				{
					write(1, &ch, 1);
					if(!compressflag)
					{
						write(sockfd, &ch, 1);
					}
				}
			}
		}
		if(fds[1].revents & POLLIN)
		{
			check3 = read(sockfd, buffer, BUFFER_SIZE);
			if(check3 < 0)
			{
				fprintf(stderr, "Error on reading input from the socket: %s\n", strerror(errno));
				tcsetattr(0, TCSANOW, &initial);
				exit(1);
			}
			if(check3 == 0)
			{
				tcsetattr(0, TCSANOW, &initial);
				exit(0);
			}
			if(logflag == 1)
			{
				char s[256];
				sprintf(s, "RECEIVED %d bytes: %s\n", check3, buffer);
				write(logfd, s, strlen(s));
			}

			if(compressflag == 1)
			{ 
				int checkinf1 = -1;
				strm2.zalloc = Z_NULL;
				strm2.zfree = Z_NULL;
				strm2.opaque = Z_NULL;

				
				checkinf1 = inflateInit(&strm2);
				if(checkinf1 != Z_OK)
				{
					inflateEnd(&strm2);
					fprintf(stderr, "inflateInit() failed!\n");
					tcsetattr(0, TCSANOW, &initial);
					exit(1);
				}
				
				strm2.avail_in = check3;
				strm2.next_in = buffer;
				strm2.avail_out = OUTBUFFER_SIZE;
				strm2.next_out = outputbuffer;
				do{
					inflate(&strm2, Z_SYNC_FLUSH);
				} while(strm2.avail_in > 0);
				inflateEnd(&strm2);
				
				ret = OUTBUFFER_SIZE - strm2.avail_out;
				for(int i=0; i< ret; i++)
				{
					ch = outputbuffer[i];
					if(outputbuffer[i] == '\n' || outputbuffer[i] == '\r') 
					{

						write(1, "\r\n", 2);
					}
					else
					{
						write(1, &ch, 1);
					}
				}

			}
			else 
			{
				for(int i=0; i<check3; i++)
				{
					ch = buffer[i];
					if(buffer[i] == '\n' || buffer[i] == '\r')
					{
						write(1, "\r\n", 2);
					}
					else
					{
						write(1, &ch, 1);
					}
				}
			}
		}
		if(fds[0].revents & (POLLHUP | POLLERR))
		{
			tcsetattr(0, TCSANOW, &initial);
			exit(0);
						
		}

		if(fds[1].revents & (POLLHUP | POLLERR))
		{
			tcsetattr(0, TCSANOW, &initial);
			exit(0);
						
		}
	}
	tcsetattr(0, TCSANOW, &initial);
	exit(0);
}


