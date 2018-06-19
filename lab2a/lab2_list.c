// NAME: Yanyin Liu

#include <stdlib.h>
#include <stdio.h>
#include <pthread.h>
#include <time.h>
#include <getopt.h>
#include <string.h>
#include <signal.h>
#include <errno.h>
#include "SortedList.c"

#define THREADS 't'
#define ITERATIONS 'i'
#define YIELD 'y'
#define SYNC 's'

#define BILLION  1000000000L

pthread_mutex_t mymutex = PTHREAD_MUTEX_INITIALIZER;

volatile int spin_lock = 0;
static int key_len =10;
char * keyset = "0123456789abcdefghijklmnopqrstuvwxyzABCDEFGHIJKLMNOPQRSTUVWXYZ";
struct threadargs{
	
	int niterate;
	char sync_option;
	SortedList_t *list;
	SortedListElement_t **elements;
};

void handler(int signo)
{
	if(signo == SIGSEGV)
	{
	        fprintf(stderr, "Caught and received SIGSEGV: %s\n", strerror(errno));
	        exit(2);
	}
}

char* get_random()
{
	
	srand((unsigned int) time (NULL));
	char *k = (char*)malloc(key_len *sizeof(char));
	for(int i=0; i<key_len; i++)
	{
		k[i] = keyset[rand()%61];
	}
	k[key_len] = '\0'; 
	return k;
}


void operation(char op, int lockflag)
{
	if(op == 'm')
	{
		if(lockflag) pthread_mutex_lock(&mymutex);
		else pthread_mutex_unlock(&mymutex);
	}
	else if(op == 's')
	{
		if(lockflag) 
		{
			while (__sync_lock_test_and_set(&spin_lock, 1));
		}
		else
		{
			__sync_lock_release(&spin_lock);
		}
	}
}

void* func(void *th_arg)
{
	struct threadargs *data;
	data = (struct threadargs*) th_arg;
	char op = data->sync_option;
	

		for(int i=0; i<data->niterate; i++)
		{
			operation(op, 1);
			
			SortedListElement_t * elements = data->elements[i];
			
			SortedList_insert(data->list, elements);
			operation(op, 0); 
		}

		operation(op, 1); 
		
		SortedList_length(data->list);
		operation(op, 0); 

		for(int i=0; i<data->niterate; i++)
		{
			operation(op, 1);
			SortedListElement_t * elements = SortedList_lookup(data->list, data->elements[i]->key);
			if(SortedList_delete(elements) == 1) 
			{
				fprintf(stderr, "Error! Fail to delete elements\n");
				exit(2);
			}
			operation(op, 0); 
		}
	
	pthread_exit(0);
}
			
int main(int argc, char **argv)
{
	int c;
	int opt_yield = 0;
	int nthread = 1, niterate = 1;
	struct timespec start, stop;
	int yieldflag = 0, syncflag=0;
	
	char sync_option = ' ';
	char *name = (char*) malloc(15*sizeof(char));
	SortedList_t *list = (SortedList_t *) malloc(sizeof(SortedList_t*));


	static struct option long_options[]=
	{
		{"threads", 1, NULL, THREADS},
		{"iterations", 1, NULL, ITERATIONS},
		{"yield", 1, NULL, YIELD},
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
				
				if(strlen(optarg) > 3)
				{
					fprintf(stderr, "Error! Invalid option for yield\n");
					exit(1);
				}
				yieldflag=1;
				size_t i;
				for(i=0; i<strlen(optarg); i++)
				{
					if (optarg[i] == 'i')
					{
						opt_yield |= INSERT_YIELD;
					}
					else if (optarg[i] == 'd') 
					{
						opt_yield |= DELETE_YIELD;
					}
					else if (optarg[i] == 'l') 
					{
						opt_yield |= LOOKUP_YIELD;
					}
					else
					{
						fprintf(stderr, "Error! Invalid option for yield\n");
						exit(1);
					}
					
				}
				break;
			case SYNC:
				sync_option = (char)* optarg;
				if((sync_option != 'm') && (sync_option != 's'))
				{
					fprintf(stderr, "Error! Invalid sync option: %s\n", strerror(errno));
					exit(1);
				}
				syncflag=1;
				break;
			default:
				fprintf(stderr, "Error! Unrecognized argument: %s\n", argv[optind-1]);
				exit(1);
		}
	}
	signal(SIGSEGV, handler);

	struct threadargs** th_args = (struct threadargs**)malloc(nthread * sizeof(struct threadargs*));
	
	for(int i=0; i<nthread; i++)
	{
		th_args[i] = (struct threadargs*) malloc(sizeof(struct threadargs));
	}
	for(int i=0; i<nthread; i++)
	{
		th_args[i]->niterate = niterate;
		th_args[i]->sync_option = sync_option;
		th_args[i]->list = list;
		
		SortedListElement_t** elementslist = (SortedListElement_t **)malloc(niterate * sizeof(SortedListElement_t*));
		
		for(int j=0; j<niterate; j++)
		{
			elementslist[j] = (SortedListElement_t *)malloc(sizeof(SortedListElement_t));
			elementslist[j]->key = get_random();
			elementslist[j]->next = NULL; 
			elementslist[j]->prev = NULL;
		}
		th_args[i]->elements = elementslist;
		
	}
	
	if(clock_gettime(CLOCK_MONOTONIC, &start) == -1)
	{
		fprintf(stderr, "Error on starting time for the run!\n");
		for(int i=0; i<nthread; i++)
		{
			free(th_args[i]);
		}
		
		free(list);
		exit(1);
	}

	pthread_t thread[nthread];
	int rc;
	for(int i=0; i<nthread; i++)
	{
		rc = pthread_create(&thread[i], NULL, func, th_args[i]);
		if(rc)
		{
			fprintf(stderr, "Error! Return code from pthread_create(): %d\n", rc);
			exit(1);
		}
	}

	for(int i=0; i<nthread; i++)
	{
		pthread_join(thread[i], NULL);
	}

	if(clock_gettime(CLOCK_MONOTONIC, &stop) == -1)
	{
		fprintf(stderr, "Error on stopping time for the run!\n");
		for(int i=0; i<nthread; i++)
		{
			free(th_args[i]);
		}
		
		free(list);
		exit(1);
	}

	if(SortedList_length(list) !=0)
	{
		fprintf(stderr, "Error! The length of list is not zero!\n");
		for(int i=0; i<nthread; i++)
		{
			free(th_args[i]);
		}
		free(list);
		exit(1);
	}

	strcpy(name, "list-");
	if(yieldflag){
		if((opt_yield & INSERT_YIELD) == 1){
			strcat(name, "i");
		}
		if((opt_yield & DELETE_YIELD) == 2){
			strcat(name, "d");
		}
		if((opt_yield & LOOKUP_YIELD) == 4){
			strcat(name, "l");
		}
	}
	else{
		strcat(name, "none");
	}
	strcat(name, "-");

	if(syncflag){
		if(sync_option == 'm'){
			strcat(name, "m");
		}
		else if(sync_option == 's'){
			strcat(name, "s");
		}
	}
	else{
		strcat(name, "none");
	}
	long long num_op = nthread * niterate * 3;
	long long runtime = BILLION * (stop.tv_sec - start.tv_sec) + (stop.tv_nsec - start.tv_nsec);
	long long avg_time_per_oper = runtime / num_op;

	printf("%s,%d,%d,%d,%lld,%lld,%lld\n", name, nthread, niterate, 1, num_op, runtime, avg_time_per_oper);
	
	for(int i=0; i<nthread; i++)
	{
		free(th_args[i]);
	}
	free(list);
	exit(0);
}
	



