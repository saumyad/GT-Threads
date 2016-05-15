/**********************************************************************
gtthread_sched.c.  

This file contains the implementation of the scheduling subset of the
gtthreads library.  A simple round-robin queue should be used.
 **********************************************************************/
/*
  Include as needed
*/

#include "gtthread.h"

/* 
   Students should define global variables and helper functions as
   they see fit.
 */

gtthread_t curr_thread_id = 0; 
gtthread_node * active_thread;
//gtthread_node * terminated_threads = NULL;
steque_t ready_queue;
//steque * waiting_queue = NULL;
steque_t terminated_queue;
struct itimerval * time_quantum;

/* Initializing signal 
     struct  sigaction {
             union __sigaction_u __sigaction_u;  signal handler 
             sigset_t sa_mask;               signal mask to apply 
             int     sa_flags;                
     };
*/
struct sigaction g_signal;
static sigset_t signal_mask;

void signal_handler(int sflag){
  // printf("Reached signal_handler\n");
  gtthread_node* next_thread = NULL;
  // printf("handler %d\n", steque_size(&ready_queue));

  sigprocmask(SIG_BLOCK, &signal_mask, NULL); 
  // printf("handler %d\n", steque_size(&ready_queue));
  if (steque_size(&ready_queue)){
      next_thread = (gtthread_node *) steque_front(&ready_queue);
  }
  if (next_thread == active_thread){
    next_thread = (gtthread_node *) steque_pop(&ready_queue);
    if (steque_size(&ready_queue)){
      next_thread = (gtthread_node *) steque_front(&ready_queue);
    }
    else{
      next_thread = NULL;
    }
 
  }
  // printf("handler just before %d\n", steque_size(&ready_queue));
  if(!next_thread)    // if no thread is ready
  {
    // printf("Next thread not found\n");
    if(active_thread->waiting)
    {   
      // thread waiting on some other thread
      printf("Active thread waiting: No Running Thread in ready queue..\n");
      exit(1);
    }
    if(!(active_thread->terminated))
    {
      // if current thread has not terminated, unblock the signal
      setitimer(ITIMER_VIRTUAL, time_quantum, NULL);
      sigprocmask(SIG_UNBLOCK, &signal_mask, NULL);
      return;
    }
    else
    {
      // all thread successfully terminated
      sigprocmask(SIG_UNBLOCK, &signal_mask, NULL);
      exit(0);
    }
  }
  else  // if there is next ready thread
  {
    // printf("handler just before front %d\n", steque_size(&ready_queue));
    // next_thread = (gtthread_node *) steque_front(&ready_queue);
    // printf("Next thread found\n");
    // if the active thread is done, pass context to next thread
    if(active_thread->terminated)
    { 
      active_thread = next_thread;
      setitimer(ITIMER_VIRTUAL, time_quantum, NULL);
      sigprocmask(SIG_UNBLOCK, &signal_mask, NULL);
      setcontext(active_thread->g_context);
    }
    else
    { // not terminated..thread is now in ready state/waiting..next time fetch from ready queue
      // printf("main thread activated\n");
      steque_enqueue(&ready_queue, active_thread);
      gtthread_node* temp = active_thread;
      active_thread = next_thread;
      // printf("%ld\n", active_thread->g_id);
      // printf("%ld\n", next_thread->g_id);
      setitimer(ITIMER_VIRTUAL, time_quantum, NULL);
      sigprocmask(SIG_UNBLOCK, &signal_mask, NULL);
      swapcontext(temp->g_context, active_thread->g_context);
    }
  }
  return;

}

void start_scheduler(){
    sigemptyset(&g_signal.sa_mask);            //initialize signal set to be empty
    g_signal.sa_flags = 0;
    g_signal.sa_handler = signal_handler;     // initialize the signal handler function
    sigaction(SIGVTALRM, &g_signal, NULL);
}

/* Creating a new thread node */
gtthread_node* gtthread_create_new(){
    gtthread_node* new_node = (gtthread_node*) malloc(sizeof(gtthread_node));
  
    if(new_node == NULL)
    {
      printf("Cannot allocate memory for new node.\n");
      return NULL;
    }
    new_node->g_id = curr_thread_id++;
    steque_init(&new_node->join_queue);
    new_node->next_waiting = NULL;
    new_node->waiting = 0;
    new_node->terminated = 0;
    new_node->g_context = NULL;
    return new_node;
}

void initialize_main_context(gtthread_node *main_thread){
  main_thread->g_context = (ucontext_t*) malloc(sizeof(ucontext_t));
  if(!(main_thread->g_context)) 
  {
    free(main_thread);
    printf("gtthread_init() Error in malloc context.\n");
    exit(1);
  }
  getcontext(main_thread->g_context);
  main_thread->g_context->uc_stack.ss_flags = 0;
  main_thread->g_context->uc_link = NULL;  // TODO not allocated stack size
  main_thread->g_context->uc_stack.ss_sp = malloc(MAX_STACK_SIZE);
  if(main_thread->g_context->uc_stack.ss_sp == NULL)
  {
    free(main_thread->g_context);
    free(main_thread);
    printf("\ngtthread_init() : FAILED on stack memory allocation main\n");
    exit(1);
  }
  main_thread->g_context->uc_stack.ss_size = MAX_STACK_SIZE;
}

void make_control_function(void*(*func1)(void*), void* arg) {
  void* retval = NULL;
  // printf("in control function\n");
  sigprocmask(SIG_UNBLOCK, &signal_mask, NULL);
  retval = func1(arg);
  gtthread_exit(retval);
}


int initialize_thread_context(gtthread_node *new_thread, void *(*start_routine)(void *), void *arg){
  new_thread->g_context = (ucontext_t*)malloc(sizeof(ucontext_t));
  if(!(new_thread->g_context))
  {
    free(new_thread);
    printf("gtthread_create() Error in malloc context \n");
    sigprocmask(SIG_UNBLOCK, &signal_mask, NULL);
    return -1;
  }
  getcontext(new_thread->g_context);
  new_thread->g_context->uc_stack.ss_sp = malloc(MAX_STACK_SIZE);
  if(new_thread->g_context->uc_stack.ss_sp == NULL)
  {
    free(new_thread->g_context);
    free(new_thread);
    printf("\ngtthread_create() : FAILED on stack memory allocation\n");
    sigprocmask(SIG_UNBLOCK, &signal_mask, NULL);
    return -1;
  }
  // TODO check if uc_link has to be null
  gtthread_node * currentThread = NULL;
  currentThread = (gtthread_node *) steque_front(&ready_queue);
  new_thread->g_context->uc_link = currentThread->g_context;
  new_thread->g_context->uc_stack.ss_flags = 0;
  new_thread->g_context->uc_stack.ss_size = MAX_STACK_SIZE;
  makecontext(new_thread->g_context, (void (*) (void))make_control_function, 2, start_routine, arg);
  return 0;
}
/*
  The gtthread_init() function does not have a corresponding pthread equivalent.
  It must be called from the main thread before any other GTThreads
  functions are called. It allows the caller to specify the scheduling
  period (quantum in micro second), and may also perform any other
  necessary initialization.  If period is zero, then thread switching should
  occur only on calls to gtthread_yield().

  Recall that the initial thread of the program (i.e. the one running
  main() ) is a thread like any other. It should have a
  gtthread_t that clients can retrieve by calling gtthread_self()
  from the initial thread, and they should be able to specify it as an
  argument to other GTThreads functions. The only difference in the
  initial thread is how it behaves when it executes a return
  instruction. You can find details on this difference in the man page
  for pthread_create.
 */
void gtthread_init(long period){

  // create main thread - the thread id should always be zero
  steque_init(&ready_queue);
  steque_init(&terminated_queue);
  // printf("passed queues\n");
  gtthread_node * main_thread = NULL;
  main_thread = gtthread_create_new();
  start_scheduler();
  if (!main_thread){
    printf("Main thread could not be created.. Exiting..\n");
    exit(1);
  }
  initialize_main_context(main_thread);
  main_thread->g_id = 0;
  active_thread = main_thread;
  steque_enqueue(&ready_queue, main_thread);
  // printf("passed queues\n");
  if (period > 0) {
      //setup signal mask
      sigemptyset(&signal_mask);
      sigaddset(&signal_mask,SIGVTALRM);

      //set up alarm
      time_quantum = (struct itimerval*) malloc(sizeof(struct itimerval));
      time_quantum->it_interval.tv_sec = 0;
      time_quantum->it_interval.tv_usec = (long) period;
      time_quantum->it_value.tv_usec = (long) period;
      time_quantum->it_value.tv_sec = 0;

      // set timer for main
      if (!(setitimer(ITIMER_VIRTUAL, time_quantum, NULL)==0)){
        printf("Timer could not be initialized.\n");
        exit(-1);
      }
      //TODO [unblock signal here??]
  }
  // else{
  //   printf("Period has to be greater than zero\n");
  //   exit(-1);
  // }

}


/*
  The gtthread_create() function mirrors the pthread_create() function,
  only default attributes are always assumed.
 */
int gtthread_create(gtthread_t *thread, void *(*start_routine)(void *), void *arg){
    gtthread_node * new_node = NULL;
    sigprocmask(SIG_BLOCK, &signal_mask, NULL);
    if((!start_routine) || (!thread))
    {
      printf("gtthread_create() Failed: Arguments invalid.\n");
      sigprocmask(SIG_UNBLOCK, &signal_mask, NULL);   
      return -1;
    } 
    new_node = gtthread_create_new();
    *thread = new_node->g_id;
    if(!new_node)
    {
      printf("gtthread_create() Failed: Cannot create Node.\n");
      sigprocmask(SIG_UNBLOCK, &signal_mask, NULL);
      return -1;
    }
    
    initialize_thread_context(new_node, start_routine, arg);
    steque_enqueue(&ready_queue, new_node);
    gtthread_node * test = NULL;
    test = (gtthread_node *) steque_front(&ready_queue);
    // printf("Enqueues %d\n", steque_size(&ready_queue));
    sigprocmask(SIG_UNBLOCK, &signal_mask, NULL);
    return 0;
}

gtthread_node * find_thread(steque_t * queue, gtthread_t *t){
//find in ready queue
  if (!steque_isempty(queue)){
    steque_node_t * it = queue->front;
      do {
        gtthread_node * item = NULL;
        item = (gtthread_node *) it->item;
        if (item->g_id == *t){
          return item;
        }
        it = it->next;
      }while(it != NULL);
  }
  return NULL;
}


/*
  The gtthread_join() function is analogous to pthread_join.
  All gtthreads are joinable.
 */
int gtthread_join(gtthread_t thread, void **status){

  gtthread_node* join_thread = NULL;
  sigprocmask(SIG_BLOCK, &signal_mask, NULL);
  join_thread = find_thread(&ready_queue, &thread);
  //thread has not terminated yet..wait
  if(join_thread)
  {
    // TODOd
    if(join_thread->waiting)
    {
      sigprocmask(SIG_UNBLOCK, &signal_mask, NULL);     
      return 1;
    }
    //TODO - nextwaiting thread?
    gtthread_node * self = NULL;
    self = (gtthread_node *) steque_pop(&ready_queue);
    steque_enqueue(&join_thread->join_queue, self);
    self->waiting = 1;
    // TODO no enqueuing required
//    steque_enqueue(&ready_queue, active_thread);
    sigprocmask(SIG_UNBLOCK, &signal_mask, NULL);
    raise(SIGVTALRM);
  }   
  else
  {
    join_thread = find_thread(&terminated_queue, &thread);
    if(join_thread)
    {
       if(join_thread->waiting)
       {
         sigprocmask(SIG_UNBLOCK, &signal_mask, NULL);
         return -1;
       }
      if(status)
      {
        *status = join_thread->retval;
      }
    }
    else
    {
      printf("Joinable Thread not found.\n");
      sigprocmask(SIG_UNBLOCK, &signal_mask, NULL);
      return 1;
    }
  }
  sigprocmask(SIG_UNBLOCK, &signal_mask, NULL);
  return 0;
}

void reschedule_join_queue(gtthread_node *f) {
  gtthread_node *next;

  while (!steque_isempty(&f->join_queue)) {
    next = (gtthread_node *) steque_pop(&f->join_queue);
    next->waiting = 0;
    steque_enqueue(&ready_queue, next);

    // if (!steque_isempty(&ready_queue)){
    //   steque_node_t * it = ready_queue.front;
    //     do {
    //       gtthread_node * item;
    //       item = (gtthread_node *) it->item;
    //       if (item->g_id == next->g_id){
    //         item->waiting = 0;
    //         break;  
    //       }
    //       it = it->next;
    //     }while(it != NULL);
    // }
  }
  return;
}

/*
  The gtthread_exit() function is analogous to pthread_exit.
 */
void gtthread_exit(void* retval){
    sigprocmask(SIG_BLOCK, &signal_mask, NULL);
    active_thread->retval = retval;
    active_thread->terminated = 1;
    steque_pop(&ready_queue);
    reschedule_join_queue(active_thread);
    steque_enqueue(&terminated_queue, active_thread);
    sigprocmask(SIG_UNBLOCK, &signal_mask, NULL);
    raise(SIGVTALRM);
}


/*
  The gtthread_yield() function is analogous to pthread_yield, causing
  the calling thread to relinquish the cpu and place itself at the
  back of the schedule queue.
 */
void gtthread_yield(void){
  // printf ("alarming\n");

  raise(SIGVTALRM);
  return;
}

/*
  The gtthread_yield() function is analogous to pthread_equal,
  returning zero if the threads are the same and non-zero otherwise.
 */
int  gtthread_equal(gtthread_t t1, gtthread_t t2){
  if (t1 == t2){
    return 1;
  }
  return 0;
}

/*
  The gtthread_cancel() function is analogous to pthread_cancel,
  allowing one thread to terminate another asynchronously.
 */
int  gtthread_cancel(gtthread_t thread){
    gtthread_node* cancel_thread = NULL; 

    sigprocmask(SIG_BLOCK, &signal_mask, NULL);
    
    cancel_thread = find_thread(&ready_queue, &thread);
    if(!cancel_thread)
    {
      printf("gtthread_cancel() Error: Node not found in ready queue.\n");
      sigprocmask(SIG_UNBLOCK, &signal_mask, NULL);
      return -1;
    }
    cancel_thread->retval = NULL; 
    cancel_thread->terminated = 1; 
    reschedule_join_queue(cancel_thread);
    steque_enqueue(&terminated_queue, cancel_thread); 
    sigprocmask(SIG_UNBLOCK, &signal_mask, NULL);
    return 0; 
}

/*
  Returns calling thread.
 */
gtthread_t gtthread_self(void){
  if(steque_size(&ready_queue)){
    gtthread_node * self_thread = NULL;
    sigprocmask(SIG_BLOCK, &signal_mask, NULL);
    self_thread = (gtthread_node *) steque_front(&ready_queue);
    sigprocmask(SIG_UNBLOCK, &signal_mask, NULL);
    return self_thread->g_id;
  }

  return -1;
  
}
