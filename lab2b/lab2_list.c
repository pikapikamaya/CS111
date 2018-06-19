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
#define LISTS 'l'
#define BILLION  1000000000L


static int key_len =10;
char * keyset = "0123456789abcdefghijklmnopqrstuvwxyzABCDEFGHIJKLMNOPQRSTUVWXYZ";
long long lock_num = 1;

long long elapsed_time=0;
struct sublists_t{
	SortedList_t *lists;
	pthread_mutex_t mymutex;
	volatile int spin_lock;
};

struct threadargs{
	int niterate;
	char sync_option;
	struct sublists_t **sub_list;
	SortedListElement_t **elements;
	long nlist;
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
	char *k = (char*)malloc((key_len+1) * sizeof(char));
    for(int i = 0; i < key_len; i++) {
        k[i] = (char)(rand() % 255 + 1);
    }
    k[key_len] = '\0';
    return k;
}
long hash(const char *key)
{
	long n = 5381;
	for(int i=0; i<key_len; i++)
	{
		n=((n << 5) + n) + key[i];
	}
	return n;
}

void operation(char op, int lockflag, struct sublists_t* sublist)
{
	struct timespec lock_start, lock_stop;
	if(op == 'm') // mutex synchronization
	{
		if(lockflag==1)
		{
			if(clock_gettime(CLOCK_MONOTONIC, &lock_start) == -1)
			{
				fprintf(stderr, "Error on starting time for the run!\n");
				exit(1);
			}
			pthread_mutex_lock(&sublist->mymutex);
			if(clock_gettime(CLOCK_MONOTONIC, &lock_stop) == -1)
			{
				fprintf(stderr, "Error on stopping time for the run!\n");
				exit(1);
			}
			lock_num++;
			//  add up the total lock acquisition time (for all threads)
			elapsed_time += BILLION * (lock_stop.tv_sec - lock_start.tv_sec) + (lock_stop.tv_nsec - lock_start.tv_nsec);
		} 
		else 
		{
			pthread_mutex_unlock(&sublist->mymutex);
		}
	}
	else if(op == 's')
	{
		if(lockflag==1) 
		{
			if(clock_gettime(CLOCK_MONOTONIC, &lock_start) == -1)
			{
				fprintf(stderr, "Error on starting time for the run!\n");
				exit(1);
			}
			while (__sync_lock_test_and_set(&sublist->spin_lock, 1));
			if(clock_gettime(CLOCK_MONOTONIC, &lock_stop) == -1)
			{
				fprintf(stderr, "Error on stopping time for the run!\n");
				exit(1);
			}
			lock_num++;
			// add up the total lock acquisition time (for all threads)
			elapsed_time += BILLION * (lock_stop.tv_sec - lock_start.tv_sec) + (lock_stop.tv_nsec - lock_start.tv_nsec);
		}
		else
		{
			__sync_lock_release(&sublist->spin_lock);
		}
	}
}


void* func(void *th_arg)
{
	struct threadargs *data;
	data = (struct threadargs*) th_arg;
	char op = data->sync_option;
	long sublist_num;
	for(int i=0; i<data->niterate; i++)
	{
		const char *key = data->elements[i]->key;
		sublist_num = ((int)hash(key)) % data->nlist;
		// in case there the sublist_num for some reasons is negative
		if(sublist_num < 0)
		{
			sublist_num = sublist_num * (-1);
		}

		operation(op, 1, data->sub_list[sublist_num]); 
		SortedListElement_t *elements = data->elements[i];
		SortedList_insert(data->sub_list[sublist_num]->lists, elements);
		operation(op, 0, data->sub_list[sublist_num]); 
	}

	for(int i=0; i<data->nlist; i++)
	{
		operation(op, 1, data->sub_list[i]); // lock
		// critical section
		// gets the list length
		if(SortedList_length(data->sub_list[i]->lists)<0)
		{
			fprintf(stderr, "Error on getting the length of sublist!!\n");
			exit(2);
		}
		operation(op, 0, data->sub_list[i]); // release the lock
	}
	

	for(int i=0; i<data->niterate; i++)
	{
		const char *key = data->elements[i]->key;
		sublist_num = ((int)hash(key)) % data->nlist;
		// in case there the sublist_num for some reasons is negative
		if(sublist_num < 0)
		{
			sublist_num = sublist_num * (-1);
		}

		operation(op, 1, data->sub_list[sublist_num]); // lock
		// critical section
		SortedListElement_t * elements = SortedList_lookup(data->sub_list[sublist_num]->lists, data->elements[i]->key);
		if(SortedList_delete(elements) == 1) // delete failed
		{
			fprintf(stderr, "Error! Fail to delete elements\n");
			exit(2);
		}
		operation(op, 0, data->sub_list[sublist_num]); // release the lock
	}
	
	// exits to re-join the parent thread
	pthread_exit(0);
}
			
int main(int argc, char **argv)
{
	int c;
	int opt_yield = 0;
	int nthread = 1, niterate = 1, nlist=1;
	struct timespec start, stop;
	int yieldflag = 0, syncflag=0;
	int len=0;
	//long long counter = 0;
	char sync_option = ' ';
	char *name = (char*) malloc(15*sizeof(char));
	//SortedList_t *list = (SortedList_t *) malloc(sizeof(SortedList_t*));


	static struct option long_options[]=
	{
		{"threads", 1, NULL, THREADS},
		{"iterations", 1, NULL, ITERATIONS},
		{"yield", 1, NULL, YIELD},
		{"sync", 1, NULL, SYNC},
		{"lists", 1, NULL, LISTS},
		{0,0,0,0}
	};

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
				// opt_yield = 1;
				//yieldopts = {none, i,d,l,id,il,dl,idl}
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
			case LISTS:
				nlist = atoi(optarg);
				if(nlist < 1)
				{
					fprintf(stderr, "Error! Invalid list number: %s\n", strerror(errno));
					exit(1);
				}
				break;
			default:
				fprintf(stderr, "Error! Unrecognized argument: %s\n", argv[optind-1]);
				exit(1);
		}
	}

	// Note that in some cases your program may not detect an error, but may simply
	// experience a segmentation fault
	signal(SIGSEGV, handler);

	// Initialize the sublist
	struct sublists_t** w_lists = (struct sublists_t**)malloc(nlist * sizeof(struct sublists_t*));
	for(int i=0; i<nlist; i++)
	{
		// struct sublists_t *s_list = w_lists[i];
		// SortedList_t *l = s_list->lists;
		// l->prev = l;
		// l->next = l;
		// l->key = NULL;
		w_lists[i] = (struct sublists_t *)malloc(sizeof(struct sublists_t));
		w_lists[i]->lists = (SortedList_t *)malloc(sizeof(SortedList_t*));
		// initialize spin lock and mutex
		w_lists[i]->spin_lock = 0;
		w_lists[i]->mymutex = (pthread_mutex_t)PTHREAD_MUTEX_INITIALIZER;
	}

	struct threadargs** th_args = (struct threadargs**)malloc(nthread * sizeof(struct threadargs*));
	// creates and initializes (with random keys) the required number (threads x iterations) of list elements.
	
	// SortedListElement_t** elementslist = (SortedListElement_t **)malloc(nthread * niterate * sizeof(SortedListElement_t*));
	for(int i=0; i<nthread; i++)
	{
		th_args[i] = (struct threadargs*) malloc(sizeof(struct threadargs));
	}
	for(int i=0; i<nthread; i++)
	{
		SortedListElement_t** elementslist = (SortedListElement_t **)malloc(niterate * sizeof(SortedListElement_t*));
		th_args[i]->niterate = niterate;
		th_args[i]->sync_option = sync_option;
		th_args[i]->sub_list = w_lists;
		th_args[i]->nlist = nlist;
		for(int j=0; j<niterate; j++)
		{
			elementslist[j] = (SortedListElement_t *)malloc(sizeof(SortedListElement_t));
			elementslist[j]->key = get_random();
			elementslist[j]->next = NULL; //empty list
			elementslist[j]->prev = NULL;
		}
		th_args[i]->elements = elementslist;
	}
	
	// return 0 for success, or -1 for failure
	if(clock_gettime(CLOCK_MONOTONIC, &start) == -1)
	{
		fprintf(stderr, "Error on starting time for the run!\n");
		for(int i=0; i<nthread; i++)
		{
			free(th_args[i]);
		}
		free(th_args);
		// free(elementslist);
		for(int i=0; i<nlist; i++)
		{
			free(w_lists[i]);
		}
		free(w_lists);
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
		// free(elementslist);
		for(int i=0; i<nlist; i++)
		{
			free(w_lists[i]);
		}
		free(th_args);
		free(w_lists);

		exit(1);
	}
	int check1;
	for(int i=0; i<nlist; i++)
	{
		check1 = SortedList_length(w_lists[i]->lists);
		if(check1 < 0)
		{
			fprintf(stderr, "Error on getting the length of the list!\n");
			for(int i=0; i<nthread; i++)
			{
				free(th_args[i]);
			}
			free(w_lists);
			//free(list);
			exit(2);
		}
		len += check1;
	}
	if(len !=0)
	{
		fprintf(stderr, "Error! The length of list is not zero!\n");
		for(int i=0; i<nthread; i++)
		{
			free(th_args[i]);
		}
		for(int i=0; i<nlist; i++)
		{
			free(w_lists[i]);
		}
		// free(elementslist);
		for(int i=0; i<nlist; i++)
		{
			free(w_lists[i]);
		}
		free(th_args);
		free(w_lists);
		exit(1);
	}

	// create the name
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
	long long avg_time_per_lock = elapsed_time / lock_num;
	printf("%s,%d,%d,%d,%lld,%lld,%lld,%lld\n", name, nthread, niterate, nlist, num_op, runtime, avg_time_per_oper, avg_time_per_lock);
	
	for(int i=0; i<nthread; i++)
	{
		free(th_args[i]);
	}
	for(int i=0; i<nlist; i++)
	{
		free(w_lists[i]);
	}
	free(th_args);
	free(w_lists);
	// free(elementslist);
	exit(0);
}




