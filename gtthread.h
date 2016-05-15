#ifndef GTTHREAD_H
#define GTTHREAD_H

#include "steque.h"
#include <stdio.h>
#include <ucontext.h>
#include <stdlib.h>
#include <signal.h>   /* define signal set and signal handler */
#include <sys/time.h> /* defines the itimerval structure that includes at least the following members:
								struct timeval it_interval timer interval
								struct timeval it_value    current value */

#define MAX_STACK_SIZE SIGSTKSZ

/* Define gtthread_t and gtthread_mutex_t types here */

// gtthread_t identifies the thread like pthread_t
typedef unsigned long gtthread_t;


// gtthread node description
typedef struct gtthread_node {
	//struct gtthread_node *
	void * retval;
	int waiting;
	int terminated;
	struct gtthread_node *next_waiting;
	gtthread_t g_id;                 //thread identifier
	ucontext_t* g_context;
	steque_t join_queue;
}gtthread_node;


// gtthread_mutex_t similar to pthread_mutex_t
typedef struct gtthread_mutex_t {
	
	long lockset;
	gtthread_t owner;

}gtthread_mutex_t; 


/* Allows the caller to specify the scheduling period (quantum in microseconds) */
void gtthread_init(long period);


int  gtthread_create(gtthread_t *thread,
                     void *(*start_routine)(void *),
                     void *arg);
int  gtthread_join(gtthread_t thread, void **status);
void gtthread_exit(void *retval);
void gtthread_yield(void);
int  gtthread_equal(gtthread_t t1, gtthread_t t2);
int  gtthread_cancel(gtthread_t thread);
gtthread_t gtthread_self(void);


int  gtthread_mutex_init(gtthread_mutex_t *mutex);
int  gtthread_mutex_lock(gtthread_mutex_t *mutex);
int  gtthread_mutex_unlock(gtthread_mutex_t *mutex);
int  gtthread_mutex_destroy(gtthread_mutex_t *mutex);

extern gtthread_node * active_thread;

#endif
