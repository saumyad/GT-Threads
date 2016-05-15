#include <stdlib.h>
#include <stdio.h>
#include <time.h>
#include <inttypes.h>
#include <pthread.h>
#include <unistd.h>

#include "philosopher.h"
#define NUM_PHILOSPHER 5

/*
 * Performs necessary initialization of mutexes.
 */
pthread_mutex_t chopsticks[NUM_PHILOSPHER];
void chopsticks_init(){
	// initialize philosphers
	int i = 0;
	
	for (i = 0; i<NUM_PHILOSPHER; i++){
		pthread_mutex_init(&chopsticks[i], NULL);	// unset all chopsticks - none of them in use
	}
	return;
}

/*
 * Cleans up mutex resources.
 */
void chopsticks_destroy(){
	int i = 0;
	for (i = 0; i<NUM_PHILOSPHER; i++){
		pthread_mutex_destroy(&chopsticks[i]);	// unset all chopsticks - none of them in use
	}
	return;

}

/*
 * Uses pickup_left_chopstick and pickup_right_chopstick
 * to pick up the chopsticks
 */   
void pickup_chopsticks(int phil_id){
    int right_c = (phil_id)%NUM_PHILOSPHER;
    int left_c = (phil_id-1 == -1 )? NUM_PHILOSPHER - 1 : phil_id-1 ;
    printf("Philospher %d is thinking...\n", phil_id);
    usleep(random() % 1000); // random backoff
    if (phil_id % 2 == 0){
    	//pick right then left
    	printf("Philosopher %d is trying to pick the right chopstick.\n", phil_id);
    	pthread_mutex_lock(&chopsticks[right_c]);
    	pickup_right_chopstick(phil_id);
    	printf("Philosopher %d is trying to pick the left chopstick.\n", phil_id);
    	pthread_mutex_lock(&chopsticks[left_c]);
    	pickup_left_chopstick(phil_id);
    }
    else{
    	printf("Philosopher %d is trying to pick the left chopstick.\n", phil_id);
    	pthread_mutex_lock(&chopsticks[left_c]);
    	pickup_left_chopstick(phil_id);
    	printf("Philosopher %d is trying to pick the right chopstick.\n", phil_id);
    	pthread_mutex_lock(&chopsticks[right_c]);
    	pickup_right_chopstick(phil_id);    
    }
    printf("Philospher %d is now eating...\n", phil_id);
    usleep(random() % 1000); // random backoff

}

/*
 * Uses pickup_left_chopstick and pickup_right_chopstick
 * to pick up the chopsticks
 */   
void putdown_chopsticks(int phil_id){
    int right_c = (phil_id)%NUM_PHILOSPHER;
    int left_c = (phil_id-1 == -1 )? NUM_PHILOSPHER - 1 : phil_id-1 ;
    printf("Philospher %d is still eating...\n", phil_id);
    usleep(random() % 1000); // random backoff   

    if (phil_id % 2 == 0){
    	//put down left then right
    	printf("Philosopher %d is trying to put down the left chopstick.\n", phil_id);
    	putdown_left_chopstick(phil_id);
    	pthread_mutex_unlock(&chopsticks[left_c]);

    	printf("Philosopher %d is trying to put down the right chopstick.\n", phil_id);
    	putdown_right_chopstick(phil_id);
    	pthread_mutex_unlock(&chopsticks[right_c]);
    }
    else{
    	printf("Philosopher %d is trying to put down the right chopstick.\n", phil_id);
    	putdown_right_chopstick(phil_id);
    	pthread_mutex_unlock(&chopsticks[right_c]);
    	printf("Philosopher %d is trying to put down the left chopstick.\n", phil_id);
    	putdown_left_chopstick(phil_id);
    	pthread_mutex_unlock(&chopsticks[left_c]);
    }
    printf("Philospher %d is now thinking again...\n", phil_id);

}
