// NAME: Yanyin Liu
// EMAIL: yanyinliu8@gmail.com
// ID: 604952257

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


#define PERIOD 'p'
#define SCALE 'c'
#define LOG 'l'

const int B = 4275;  // B value of the thermistor
const int R0 = 100000;            // R0 = 100k

// set default values
static int periods=1;
static char scale = 'F';
static char filename[256];
static FILE* filefd = 0;

static int shutdown = 0;
static int stop = 0;
static int logflag=0;

mraa_aio_context temp_sensor;
mraa_gpio_context button;


void interrupt_signal()
{
	shutdown = 1;
}

double get_temperature()
{
	int reading = mraa_aio_read(temp_sensor);
	double t = 1023.0 / ((double)reading - 1.0);
	t = R0 * t;
	double C = 1.0/(log(t/R0)/B + 1/298.15) - 273.15;
	if(scale == 'F')
	{
		return ((C * 9)/5 + 32);
	}
	else
	{
		return C;
	}

}

int checkcmd(const char* s1, const char* s2)
{
	int len1 = strlen(s1);
	int len2 = strlen(s2);
	//All received commands will be terminated by a new-line character ('\n').
	if(len1 != len2)
	{
		return 0; // lengthes are not matched
	}
	int i=0;
	for(; i<len2; i++)
	{
		if(s1[i] != s2[i]) return 0;
	}
	return 1;
}


void command_exe(const char* commands)
{
	// check for each commands from stdin
	if(checkcmd(commands, "SCALE=F"))
	{
		scale = 'F';
		if (logflag)
		{
			fprintf(filefd, "%s\n", commands);
		}
		fflush(filefd);
	}
	else if(checkcmd(commands, "SCALE=C"))
	{
		scale = 'C';
		if (logflag)
		{
			fprintf(filefd, "%s\n", commands);
		}
		fflush(filefd);
	}
	else if(checkcmd(commands, "STOP"))
	{
		stop = 1;
		if (logflag)
		{
			fprintf(filefd, "%s\n", commands);
		}
		fflush(filefd);
	}
	else if(checkcmd(commands, "START"))
	{
		stop = 0;
		if (logflag)
		{
			fprintf(filefd, "%s\n", commands);
		}
		fflush(filefd);
	}
	else if(checkcmd(commands, "OFF"))
	{
		if (logflag)
		{
			fprintf(filefd, "%s\n", commands);
		}
		fflush(filefd);
		shutdown=1;
	} //PERIOD=seconds
	else if((strlen(commands) > 7) && (commands[0]== 'P') && (commands[1]== 'E') && (commands[2]== 'R') && (commands[3]== 'I')
		&& (commands[4]== 'O') && (commands[5]== 'D') && (commands[6]== '='))
	{
		int length = strlen(commands) - 1;
		int i = 7;
		while(i<length)
		{
			if(!isdigit(commands[i]))
			{
				fprintf(stderr, "Invalid period command / argument received!\n");
				exit(1);
			}
		}
		int newperiod = (int)atoi(commands+7);
		periods = newperiod;
		if (logflag)
		{
			fprintf(filefd, "%s\n", commands);
		}
		fflush(filefd);
	}
	else if(strlen(commands) > 4 && (commands[0]== 'L') && (commands[1]== 'O') && (commands[2]== 'G') && (commands[3]== ' '))
	{
		if(logflag == 1)
		{
			fputs(commands, filefd);
		}
		else return;
	}
	else
	{
		fprintf(stderr, "Invalid command received!\n");
		exit(1);
	}
	return;
}

int main(int argc, char **argv)
{
	char commands[1024];
	memset(commands, 0, 1024);
	char inputs[1024];
	memset(inputs, 0, 1024);
	static struct option long_options[]=
	{
		{"period", 1, NULL, PERIOD},
		{"scale", 1, NULL, SCALE},
		{"log", 1, NULL, LOG},
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
					fprintf(stderr, "Usage: lab4b [--period=#] [--scale=[CF]] [--log=filename]\n");
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
					fprintf(stderr, "Usage: lab4b [--period=#] [--scale=[CF]] [--log=filename]\n");
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
			default:
				fprintf(stderr, "Error! Unrecognized argument: %s\n", argv[optind-1]);
				fprintf(stderr, "Usage: lab4b [--period=#] [--scale=[CF]] [--log=filename]\n");
				exit(1);
		}
	}

	//Initialize GPIO / AIO
	temp_sensor = mraa_aio_init(1);
	// used GPIO pin 50 and yet initialized pin 60 in your C code
	button = mraa_gpio_init(60);
	mraa_gpio_dir(button, MRAA_GPIO_IN); // Set Gpio(s) direction
	mraa_gpio_isr(button, MRAA_GPIO_EDGE_RISING, &interrupt_signal, NULL); //Set an interrupt on pin(s).

	// 
	struct timeval clk;
	time_t next_sample;
	struct tm *time;

	// sensor polling
	struct pollfd fds[1];
	fds[0].fd = STDIN_FILENO;
	fds[0].events = POLLIN | POLLHUP | POLLERR;
	fds[0].revents = 0;
	char buffer[128];
	while(!shutdown)
	{
		if(gettimeofday(&clk, 0) != 0)
		{
			fprintf(stderr, "Failure on gettimeofday()!\n");
			exit(1);
		}
		double temperature = get_temperature();
		if(!stop && (clk.tv_sec >= next_sample))
		{
			time = localtime(&clk.tv_sec);
			int dec_temp = temperature * 10; 
			// print out the sample report
			// for temperature: a decimal temperature in degrees and tenths (e.g. 98.6)
			// %02d: "format the integer with 2 digits, left padding it with zeroes
			sprintf(buffer, "%02d:%02d:%02d %d.%1d\n", time->tm_hour, time->tm_min, time->tm_sec, dec_temp/10, dec_temp%10);

			// fprintf(stdout, "%02d:%02d:%02d %d.%1d\n", time->tm_hour, time->tm_min, time->tm_sec, dec_temp/10, dec_temp%10);
			fputs(buffer, stdout);
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
			exit(1);
		}
		if(c > 0)
		{
			if(fds[0].revents & POLLIN)
			{
				// fgets() reads a line from the specified stream and stores it into the string pointed to by str. 
				// fgets(commands, 256, stdin);
				memset(inputs, 0, 1024);
				int r = read(STDIN_FILENO, inputs, 1024);
				if(r < 0)
				{
					fprintf(stderr, "Error on reading from stdin: %s\n", strerror(errno));
					exit(1);
				}
				int i=0;
				for(; i<r; i++)
				{
					// All received commands will be terminated by a new-line character ('\n'). 
					if(inputs[i] == '\n')
					{
						command_exe(commands);
						memset(commands, 0, 1024);
					}
					else
					{
						strncat(commands, &inputs[i], 1);
					}
				}
			}
		}
	}

	time = localtime(&clk.tv_sec);
	// print out the sample report
	// output (and log) a time-stamped SHUTDOWN message and exit.
	//fprintf(stdout, "%02d:%02d:%02d SHUTDOWN\n", time->tm_hour, time->tm_min, time->tm_sec);

	sprintf(buffer,"%02d:%02d:%02d SHUTDOWN\n", time->tm_hour, time->tm_min, time->tm_sec);
	fputs(buffer, stdout);
	if(logflag == 1)
	{
	// 	fprintf(filefd, "%02d:%02d:%02d SHUTDOWN\n", time->tm_hour, time->tm_min, time->tm_sec);
		fputs(buffer, filefd);
	}

	mraa_aio_close(temp_sensor);
	mraa_gpio_close(button);
	exit(0);
}
