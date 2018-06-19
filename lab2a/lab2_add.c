// NAME: Yanyin Liu

#include <stdlib.h>
#include <stdio.h>
#include <pthread.h>
#include <time.h>
#include <getopt.h>
#include <string.h>
#include <errno.h>
#include <sched.h> 
#define THREADS 't'
#define ITERATIONS 'i'
#define YIELD 'y'
#define SYNC 's'

#define BILLION  1000000000L

// void add(long long *pointer, long long value) {
//     long long sum = *pointer + value;
//     *pointer = sum;
// }

//  initialize a mutex variable:
pthread_mutex_t mymutex = PTHREAD_MUTEX_INITIALIZER;
int lock = 0;
int nthread = 1, niterate = 1;
struct timespec start, stop;

struct threadargs{
	long long *counter;
	int niterate;
	char sync_option;
};

int opt_yield;
void add(long long *pointer, long long value) {
    long long sum = *pointer + value;
    if (opt_yield)
            sched_yield();
    *pointer = sum;
}

void m_add(long long *pointer, long long value)
{
	pthread_mutex_lock(&mymutex);
	long long sum = *pointer + value;
    if (opt_yield)
            sched_yield();
    *pointer = sum;
	pthread_mutex_unlock(&mymutex);
}

void s_add(long long *pointer, long long value)
{
	while (__sync_lock_test_and_set(&lock, 1));
	long long sum = *pointer + value;
    if (opt_yield)
            sched_yield();
    *pointer = sum;
	__sync_lock_release(&lock);
}

void c_add(long long *pointer, long long value)
{
	int new, old;
	do
	{
		old = *pointer;
		if (opt_yield)
            sched_yield();
		new = old + value;
	}while(__sync_val_compare_and_swap(pointer, old, new) != old);
}

void* my_add(void *th_args)
{
	struct threadargs *data = (struct threadargs*) th_args;
	long long *counter = data->counter;
	int niterate = data->niterate;
	char sync_option = data->sync_option;
	//long long old, new;
	int i,j;
	for (i=0; i< niterate; i++)
	{
		if((sync_option) == 'm')
		{
			m_add(counter, 1);
		}
		else if(sync_option == 's')
		{
			s_add(counter, 1);
		}
		else if(sync_option == 'c')
		{
			c_add(counter, 1);
		}
		else if(sync_option == ' ')
		{
			add(counter, 1);
		}
	}


	for (j=0; j< (data->niterate); j++)
	{
		if((data->sync_option) == 'm')
		{
			m_add(counter, -1);
		}
		else if((data->sync_option) == 's')
		{
			s_add(counter, -1);
		}
		else if((data->sync_option) == 'c')
		{
			c_add(counter, -1);
		}
		else if((data->sync_option) == ' ')
		{
			add(counter, -1);
		}
	}
	// exits to re-join the parent thread
	pthread_exit(0);
}


int main(int argc, char **argv)
{	
	// int nthread = 1, niterate = 1;
	// struct timespec start, stop;
	//int opt; // use to get the option characters
	// int yieldflag = 0;
	long long counter = 0;
	char sync_option = ' ';
	int c,i;
	//char *name = (char*) malloc(32*sizeof(char));
	static struct option long_options[]=
	{
		{"threads", 1, NULL, THREADS},
		{"iterations", 1, NULL, ITERATIONS},
		{"yield", 0, NULL, YIELD},
		{"sync", 1, NULL, SYNC},
		{0,0,0,0}
	};
	while(1)
	{
		c = getopt_long(argc, argv, "", long_options, NULL);
		if(c == -1)
		{
			break;
		}
		//fprintf(stderr, "c: %c, optind: %d, optarg: %s\n", c, optind, optarg);
		switch(c)
		{
			case THREADS:
				nthread = atoi(optarg);
				if(nthread < 1)
				{
					fprintf(stderr, "Error! Invalid thread number: %s\n", strerror(errno));
					exit(1);
				}
				break;
			case ITERATIONS: 
				niterate = atoi(optarg);
				if(niterate < 1)
				{
					fprintf(stderr, "Error! Invalid iteration number: %s\n", strerror(errno));
					exit(1);
				}
				break;
			case YIELD:
				opt_yield = 1;
				break;
			case SYNC:
				sync_option = (char)* optarg;
				if((sync_option != 'm') && (sync_option != 's') && (sync_option != 'c'))
				{
					fprintf(stderr, "Error! Invalid sync option: %s\n", strerror(errno));
					exit(1);
				}
				
				
				break;
			default:
				fprintf(stderr, "Error! Unrecognized argument: %s\n", argv[optind-1]);
				exit(1);
		}
	}


	
	struct threadargs th_args;
	th_args.counter = &counter;
	th_args.niterate = niterate;
	th_args.sync_option = sync_option;

	if(clock_gettime(CLOCK_MONOTONIC, &start) == -1)
	{
		fprintf(stderr, "Error on starting time for the run!\n");
		exit(1);
	}

	pthread_t thread[nthread];
	int rc;
	for(i=0; i<nthread; i++)
	{
		rc = pthread_create(&thread[i], NULL, my_add, (void*) &th_args);
		if(rc)
		{
			fprintf(stderr, "Error! Return code from pthread_create(): %d\n", rc);
			exit(1);
		}
	}

	for(i=0; i<nthread; i++)
	{
		pthread_join(thread[i], NULL);
	}

	if(clock_gettime(CLOCK_MONOTONIC, &stop) == -1)
	{
		fprintf(stderr, "Error on stopping time for the run!\n");
		exit(1);
	}

	//prints to stdout a comma-separated-value (CSV)
	// name = "add-none";
	switch(sync_option)
	{
		case 'm': 
			if(opt_yield)
			{
				fprintf(stdout, "add-yield-m,");
			}
			else
			{
				fprintf(stdout, "add-m,");
			}
			break;
		case 's':
			if(opt_yield)
			{
				fprintf(stdout, "add-yield-s,");
			}
			else
			{
				fprintf(stdout,"add-s,");
			}
			break;
		case 'c':
			if(opt_yield)
			{
				fprintf(stdout,"add-yield-c,");
			}
			else
			{
				fprintf(stdout, "add-c,");
			}
			break;
		// case ' ':
		// 	if(opt_yield)
		// 	{
		// 		name = "add-yield-none";
		// 	}
		// 	else
		// 	{
		// 		name = "add-none";
		// 	}
		// 	break;
		default: 
			if(opt_yield)
			{
				fprintf(stdout,"add-yield-none,");
			}
			else
			{
				fprintf(stdout,"add-none,");
			}
	}

	long long num_op = nthread * niterate * 2;
	long long runtime = BILLION * (stop.tv_sec - start.tv_sec) + (stop.tv_nsec - start.tv_nsec);
	long long avg_time_per_oper = runtime / num_op;

	printf("%d,%d,%lld,%lld,%lld,%lld\n", nthread, niterate, num_op, runtime, avg_time_per_oper, counter);
	if(counter != 0)
		exit(2);

	exit(0);
}



