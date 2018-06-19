// CS 111, Project 0
// NAME: Yanyin Liu

#include <stdio.h>
#include <stdlib.h>
#include <getopt.h> // header file for getopt_long()
#include <signal.h> // header file for signal()
#include <sys/stat.h>
#include <fcntl.h> // header for create the file
#include <errno.h>
#include <string.h>
#include <unistd.h> // for size_t
#include <sys/types.h>

#define INPUT 'i'
#define OUTPUT 'o'
#define SEGFAULT 's'
#define CATCH 'c'

void handler(int signo)
{
        // these will write to the ouput file...
        // char message[31] = "Caught and received SIGSEGV!\n";
        // write(1, message, 34);
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
                // name of long option, number of argument, flag, value to return
                {"input", 1 , NULL, INPUT},
                {"output", 1, NULL, OUTPUT},
                {"segfault", 0, NULL, SEGFAULT},
                {"catch", 0, NULL, CATCH},
                {0, 0, 0, 0}
        };

        /*
        int getopt_long(int argc, char * const argv[],
                  const char *optstring,
                  const struct option *longopts, int *longindex);

    If the return value is -1, which means all command-line options have been parsed
    For optstring, only input and output options requires argument,
    so i and o followed by a colon
        */
        int Segment;
        int c; // get the option charactes
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
                                        // creates a copy of a file descriptor for input file
                                        //close(0);
                                        dup2(fd0,0);
                                        close(fd0);
                                }
                                break;

                        case OUTPUT:
                                // set the mode for create the output file, with read and write and execute
                                // permissions for the file owner and read permission for group and others.
                                //mode_t mode = S_IRWXU | S_IRGRP | S_IROTH;
                                fd1 = creat(optarg, S_IRWXU | S_IRGRP | S_IROTH);
                                if(fd1<0)
                                {
                                        fprintf(stderr, "Error! Unable to create and open output file: %s\nCause: %s\n",optarg, strerror(errno));
                                        exit(3);
                                }
                                else
                                {
                                        // creates a copy of a file descriptor for output file
                                        //close(1);
                                        dup2(fd1,1);
                                        close(fd1);
                                }
                                break;

                        case SEGFAULT:
                                Segment = 1;
                                break;

                        case CATCH:
                                // void ( *signal(int signum, void (*handler)(int)) ) (int);
                                signal(SIGSEGV, handler); // catch the segmentation fault
                                break;
                        default:
                                // fprintf(stderr, "Error! Unrecognized argument: %s\n", strerror(errno));
                                fprintf(stderr, "Error! Unrecognized argument: %s\n", argv[optind-1]);
                                exit(1);
                }
        }


        // If this argument is specified, do it immediately, and do not copy from stdin to stdout.
        //char *nullptr = NULL;
        if(Segment)
        {
                char *c = NULL;
                *c='s';
        }

        // if no segfault was caused, copy stdin to stdout
        char buf;
        ssize_t r = read(0, &buf, 1);
        while (r>0)
        {
                write(1, &buf, 1);
                r = read(0, &buf, 1);
        }

        exit(0); // copy successful

}


