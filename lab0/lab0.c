// CS 111, Project 0
// NAME: Yanyin Liu

#include <stdio.h>
#include <stdlib.h>
#include <getopt.h> 
#include <signal.h> 
#include <sys/stat.h>
#include <fcntl.h>
#include <errno.h>
#include <string.h>
#include <unistd.h>
#include <sys/types.h>

#define INPUT 'i'
#define OUTPUT 'o'
#define SEGFAULT 's'
#define CATCH 'c'

void handler(int signo)
{
        if(signo == SIGSEGV)
        {
                fprintf(stderr, "Caught and received SIGSEGV: %s\n", strerror(errno));
                exit(4);
        }
}


int main(int argc, char **argv)
{
        //an array of struct option declared in <getopt.h>
        static struct option long_options[]=
        {
                
                {"input", 1 , NULL, INPUT},
                {"output", 1, NULL, OUTPUT},
                {"segfault", 0, NULL, SEGFAULT},
                {"catch", 0, NULL, CATCH},
                {0, 0, 0, 0}
        };

   
        int Segment;
        int c; 
        int fd0;
        int fd1;
        while(1)
        {
                c = getopt_long(argc, argv, "", long_options, NULL);
                if(c == -1)
                {
                        break;
                }
                fprintf(stderr, "c: %c, optind: %d, optarg: %s\n", c, optind, optarg);
                switch(c)
                {
                        case INPUT:
                                fd0 = open(optarg, O_RDONLY);
                                if(fd0<0)
                                {
                                        fprintf(stderr, "Error! Unable to open input file: %s\nCause: %s\n",optarg, strerror(errno));
                                        exit(2);
                                }
                                else
                                {
                                        dup2(fd0,0);
                                        close(fd0);
                                }
                                break;

                        case OUTPUT:
                              
                                fd1 = creat(optarg, S_IRWXU | S_IRGRP | S_IROTH);
                                if(fd1<0)
                                {
                                        fprintf(stderr, "Error! Unable to create and open output file: %s\nCause: %s\n",optarg, strerror(errno));
                                        exit(3);
                                }
                                else
                                {
                                       
                                        dup2(fd1,1);
                                        close(fd1);
                                }
                                break;

                        case SEGFAULT:
                                Segment = 1;
                                break;

                        case CATCH:
                               
                                signal(SIGSEGV, handler); 
                                break;
                        default:
                                
                                fprintf(stderr, "Error! Unrecognized argument: %s\n", argv[optind-1]);
                                exit(1);
                }
        }


       
        if(Segment)
        {
                char *c = NULL;
                *c='s';
        }

       
        char buf;
        ssize_t r = read(0, &buf, 1);
        while (r>0)
        {
                write(1, &buf, 1);
                r = read(0, &buf, 1);
        }

        exit(0);
}


