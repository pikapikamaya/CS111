// NAME: Yanyin Liu
// EMAIL: yanyinliu8@gmail.com
// ID: 604952257

// remember to remove print to stdout before submit

#include <stdlib.h>
#include <stdio.h>
#include <mraa.h>
#include <math.h>
#include <time.h>
#include <getopt.h>
#include <string.h>
#include <signal.h>
#include <errno.h>
#include <poll.h>
#include <unistd.h> 
#include <sys/time.h>
#include <sys/types.h>
#include <ctype.h>
#include <sys/errno.h>
#include <mraa/aio.h>
#include <mraa/gpio.h>
#include <netinet/in.h>
#include <netdb.h>
#include <sys/socket.h>

#define PERIOD 'p'
#define SCALE 'c'
#define LOG 'l'
#define ID 'i'
#define HOST 'h'


// set default values
int periods=1;
char scale = 'F';
char filename[256];
FILE* filefd = 0;

int shutdown_flag = 0;
int stop = 0;
int logflag = 0;
int idflag=0;
int hostflag=0;

char *id;
char host[256];
int port = 0;
//static char id_string[13] = "ID=";

int socketfd;
struct hostent* server;
struct sockaddr_in server_addr;

mraa_aio_context temp_sensor;

double get_temperature()
{
	int reading = mraa_aio_read(temp_sensor);
	double t = 1023.0 / ((double)reading) - 1.0;
	int B = 4275;  // B value of the thermistor
	t *= 100000.0;
	double C = 1.0 / (log(t/100000.0)/B + 1/298.15) - 273.15;
	if(scale == 'F')
	{
		return C * 9/5 + 32;
	}
	else
	{
		return C;
	}
}

int checkcmd(const char* s1, const char* s2)
{
	int len1 = strlen(s1); // received commands
	int len2 = strlen(s2); // string to compare
	//All received commands will be terminated by a new-line character ('\n').
	if(len1 != len2 + 1)
	{
		return 0; // lengthes are not matched
	}
	int i=0;
	for(; i<len2; i++)
	{
		if(s1[i] == '\n') 
		{
			break;
		}
		if(s1[i] != s2[i]) 
		{
			return 0;
		}
	}
	return 1;
}

void command_exe(const char* commands){
	if(checkcmd(commands, "SCALE=F")) 
	{ 
		scale = 'F'; 
		if (logflag)
		{
			fprintf(filefd, "%s", commands);
		}
		fflush(filefd);
	}
	else if(checkcmd(commands, "SCALE=C"))
	{
		scale = 'C'; 
		if (logflag)
		{
			fprintf(filefd, "%s", commands);
		}
		fflush(filefd);
	}
	else if(checkcmd(commands, "STOP")) 
	{ 
		stop = 1; 
		if (logflag)
		{
			fprintf(filefd, "%s", commands);
		}
		fflush(filefd);
	}
	else if(checkcmd(commands, "START")) 
	{ 
		stop = 0; 
		if (logflag)
		{
			fprintf(filefd, "%s", commands);
		}
		fflush(filefd);
	}
	else if(checkcmd(commands, "OFF"))
	{
		shutdown_flag=1;
		if (logflag)
		{
			fprintf(filefd, "%s", commands);
		}
		fflush(filefd);
	}
	else if (strlen(commands) > 4 && (commands[0]== 'L') && (commands[1]== 'O') && (commands[2]== 'G') && (commands[3]== ' '))
	{
		if(logflag == 1)
		{
			fputs(commands, filefd);
		}
		else return;
	}
	else if((strlen(commands) > 7) && (commands[0]== 'P') && (commands[1]== 'E') && (commands[2]== 'R') && (commands[3]== 'I')
		&& (commands[4]== 'O') && (commands[5]== 'D') && (commands[6]== '='))
	{
		int i = 0;
		char *checkstr = "PERIOD=";
		// check if it mactches the required string
		while (commands[i] != '\0' && checkstr[i] != '\0')
		{
			if (commands[i] != checkstr[i]) // wrong argutment
			{
				fprintf(stderr, "Invalid period command / argument received!\n");
				exit(1);
			}
			i++;
		}
		if (i != 7) // up to this point, should match the string
		{
			fprintf(stderr, "Invalid period command / argument received!\n");
			exit(1);
		}
		int j = strlen(commands) - 1;
		while (i < j)
		{
			if (!isdigit(commands[i]))
			{
				fprintf(stderr, "Invalid period command / argument received!\n");
				exit(1);
			}
			i++;
		}
		periods = (int)atoi(commands+7);
		if (logflag)
		{
			fprintf(filefd, "%s", commands);
		}
		fflush(filefd);
	}
	else
	{
		fprintf(stderr, "Invalid command received!\n");
		exit(1);
	}
}

int main(int argc, char **argv)
{
	// char commands[1024];
	// memset(commands, 0, 1024);
	char inputs[1024];
	memset(inputs, 0, 1024);
	static struct option long_options[]=
	{
		{"period", 1, NULL, PERIOD},
		{"scale", 1, NULL, SCALE},
		{"log", 1, NULL, LOG},
		{"id", 1, NULL, ID},
		{"host", 1, NULL, HOST},
		{0,0,0,0}
	};
	int c;
	//int opt; // use to get the option characters
	while(1)
	{
		c = getopt_long(argc, argv, "", long_options, NULL);
		if(c == -1)
		{
			break;
		}
		switch(c)
		{
			case PERIOD:
				periods = atoi(optarg);
				if(periods<1)
				{
					fprintf(stderr, "Error! Period has to be bigger than or equal to 1!\n");
					fprintf(stderr, "Usage: lab4c_tcp port-number [--id=9-digit-number] [--host=name or address] [--period=#] [--scale=[CF]] [--log=filename]\n");
					exit(1);
				}
				break;
			case SCALE:
				if(*optarg == 'F' || *optarg == 'C')
				{
					scale = *optarg;
				}
				else
				{
					fprintf(stderr, "Error! Invalid argument for scale!\n");
					fprintf(stderr, "Usage: lab4c_tcp port-number [--id=9-digit-number] [--host=name or address] [--period=#] [--scale=[CF]] [--log=filename]\n");
					exit(1);
				}
				break;
			case LOG:
				logflag = 1;
				strcpy(filename, optarg);
				filefd = fopen(filename, "a+");
				// fail ==>NULL is returned and the global variable errno is set to indicate the error.
				if(filefd==NULL)
				{
					fprintf(stderr, "Error on opening the file!\n");
					exit(1);
				}
				break;
			case ID:
				if(strlen(optarg) != 9)
				{
					fprintf(stderr, "Error! id=9-digit-number!\n");
					fprintf(stderr, "Usage: lab4c_tcp port-number [--id=9-digit-number] [--host=name or address] [--period=#] [--scale=[CF]] [--log=filename]\n");
					exit(1);
				}
				idflag = 1;
				// strcpy(id, optarg);
				// strcat(id_string, optarg);
				// strcat(id_string, "\n");
				// sprintf(id, "ID=%s\n", optarg);
				// write_to_log(id);
				id = optarg;
				break;
			case HOST:
				hostflag = 1;
				strcpy(host, optarg);
				break;
			default:
				fprintf(stderr, "Error! Unrecognized argument: %s\n", argv[optind-1]);
				fprintf(stderr, "Usage: lab4c_tcp port-number [--id=9-digit-number] [--host=name or address] [--period=#] [--scale=[CF]] [--log=filename]\n");
				exit(1);
		}
	}
	// if(logflag == 1)
	// {
	// 	fputs(id_string,filefd); //immediately send (and log) an ID terminated with a newline
	// }

	if((optind == argc) || (hostflag == 0) || (idflag == 0))
	{
		fprintf(stderr, "Error! Missing argument!\n");
		fprintf(stderr, "Usage: lab4c_tcp port-number [--id=9-digit-number] [--host=name or address] [--period=#] [--scale=[CF]] [--log=filename]\n");
		exit(1);
	}
	port = atoi(argv[optind]);
	if(port <= 0)
	{
		fprintf(stderr, "Invalid port number!\n");
		fprintf(stderr, "Usage: lab4c_tcp port-number [--id=9-digit-number] [--host=name or address] [--period=#] [--scale=[CF]] [--log=filename]\n");
		exit(1);
	}
	if(host == NULL)
	{
		fprintf(stderr, "Invalid host!\n");
		fprintf(stderr, "Usage: lab4c_tcp port-number [--id=9-digit-number] [--host=name or address] [--period=#] [--scale=[CF]] [--log=filename]\n");
		exit(1);
	}

	char tempbuf[20];
	sprintf(tempbuf, "ID=%s\n", id);
	if(logflag)
	{
		fputs(tempbuf, filefd);
	}
	// setup socket
	socketfd = socket(AF_INET, SOCK_STREAM, 0);
	if(socketfd < 0)
	{
		fprintf(stderr, "Error on opening socket\n");
		exit(2);
	}
	server = gethostbyname(host);
	if(server == NULL)
	{
		fprintf(stderr, "Error on getting host by name\n");
		exit(2);
	}
	memset((char*) &server_addr, 0, sizeof(server_addr)); 
	server_addr.sin_family = AF_INET;
	// void *memmove(void *dest, const void *src, size_t n);
	// memmove(&server_addr.sin_addr.s_addr, server->h_addr, server->h_length);
	bcopy((char *)server->h_addr, (char *) &server_addr.sin_addr.s_addr, server->h_length);
	server_addr.sin_port = htons(port);
	// establishs a connection to the server
	if(connect(socketfd, (struct sockaddr *)&server_addr, sizeof(server_addr)) < 0)
	{
		fprintf(stderr, "Error! Fail on binding!\n");
		exit(2);
	}
	dprintf(socketfd, "ID=%s\n", id);

	// write_to_server(id);
	//Initialize GPIO / AIO
	temp_sensor = mraa_aio_init(1);

	struct timeval clk;
	time_t next_sample=0;
	struct tm *time;

	// sensor polling
	struct pollfd fds[1];
	fds[0].fd = socketfd;
	fds[0].events = POLLIN | POLLHUP | POLLERR;
	fds[0].revents = 0;


	char buffer[128];
	while(!shutdown_flag)
	{
		if(gettimeofday(&clk, 0) != 0)
		{
			fprintf(stderr, "Failure on gettimeofday()!\n");
			exit(2);
		}
		// fprintf(stdout, "stop flog is: %d", stop);
		double temperature = get_temperature();
		if(!stop && (clk.tv_sec >= next_sample))
		{
			time = localtime(&(clk.tv_sec));
			// int dec_temp = temperature * 10; 
			// print out the sample report
			// for temperature: a decimal temperature in degrees and tenths (e.g. 98.6)
			// %02d: "format the integer with 2 digits, left padding it with zeroes
			sprintf(buffer, "%02d:%02d:%02d %.1f\n", time->tm_hour, time->tm_min, time->tm_sec, temperature);
			dprintf(socketfd, "%02d:%02d:%02d %.1f\n", time->tm_hour, time->tm_min, time->tm_sec, temperature);
			// fprintf(stdout, "%02d:%02d:%02d %d.%1d\n", time->tm_hour, time->tm_min, time->tm_sec, dec_temp/10, dec_temp%10);
			// fputs(buffer, stdout);
			//fprintf(stdout, "logflag is: %d", logflag);
			if(logflag == 1)
			{
				// fprintf(filefd, "%02d:%02d:%02d %d.%1d\n", time->tm_hour, time->tm_min, time->tm_sec, dec_temp/10, dec_temp%10);
				fputs(buffer, filefd);
			}
			next_sample = clk.tv_sec + periods;
		}

		int c = poll(fds, 1, 0);
		if(c < 0)
		{
			fprintf(stderr, "Ooh.. poll() failed: %s\n", strerror(errno));
			exit(2);
		}
		
		if(fds[0].revents & POLLIN)
		{
			// fgets() reads a line from the specified stream and stores it into the string pointed to by str. 
			// fgets(commands, 256, stdin);
			// read from socket
			// FILE *fdopen(int fd, const char *mode); ==> mode="r" means Open text file for reading
			char commands[100];
			memset(commands, 0, 100);
			// fdopen mode: r ==> Open text file for reading. The stream is positioned at the beginning of the file.
			FILE *r = fdopen(socketfd, "r");
			fgets(commands, 100, r);
			command_exe(commands);

		}
		
	}
	gettimeofday(&clk, 0);
	time = localtime(&clk.tv_sec);
	// print out the sample report
	// output (and log) a time-stamped SHUTDOWN message and exit.
	//fprintf(stdout, "%02d:%02d:%02d SHUTDOWN\n", time->tm_hour, time->tm_min, time->tm_sec);

	sprintf(buffer,"%02d:%02d:%02d SHUTDOWN\n", time->tm_hour, time->tm_min, time->tm_sec);
	dprintf(socketfd,"%02d:%02d:%02d SHUTDOWN\n", time->tm_hour, time->tm_min, time->tm_sec);
	// fputs(buffer, stdout);
	if(logflag == 1)
	{
	// 	fprintf(filefd, "%02d:%02d:%02d SHUTDOWN\n", time->tm_hour, time->tm_min, time->tm_sec);
		fputs(buffer, filefd);
	}

	mraa_aio_close(temp_sensor);
	// mraa_gpio_close(button);
	exit(0);
}
