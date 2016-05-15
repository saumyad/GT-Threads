/**********************************************************************
gtthread_mutex.c.  

This file contains the implementation of the mutex subset of the
gtthreads library.  The locks can be implemented with a simple queue.
 **********************************************************************/

/*
  Include as needed
*/


#include "gtthread.h"
static sigset_t signal_mask;
/*
  The gtthread_mutex_init() function is analogous to
  pthread_mutex_init with the default parameters enforced.
  There is no need to create a static initializer analogous to
  PTHREAD_MUTEX_INITIALIZER.
 */
int gtthread_mutex_init(gtthread_mutex_t* mutex){
  if (mutex->lockset == 1) {
    return -1;
  }
  sigemptyset(&signal_mask);
  sigaddset(&signal_mask,SIGVTALRM);
  mutex->lockset = 0;
  mutex->owner = -1;
  return 0;

}

/*
  The gtthread_mutex_lock() is analogous to pthread_mutex_lock.
  Returns zero on success.
 */
int gtthread_mutex_lock(gtthread_mutex_t* mutex){
  if( ( mutex->owner ) == active_thread->g_id )
  {
    printf("Current Thread is already the owner of lock.\n");
    return -1;
  }
  sigprocmask(SIG_BLOCK, &signal_mask, NULL);

  while( mutex->lockset != 0 && mutex->owner != active_thread->g_id )
  {
    sigprocmask(SIG_UNBLOCK, &signal_mask, NULL);
    gtthread_yield();
    sigprocmask(SIG_BLOCK, &signal_mask, NULL);
  }
  mutex->lockset = 1;
  mutex->owner = active_thread->g_id;
  sigprocmask(SIG_UNBLOCK, &signal_mask, NULL);
  return 0;
}
/*
  The gtthread_mutex_unlock() is analogous to pthread_mutex_unlock.
  Returns zero on success.
 */
int gtthread_mutex_unlock(gtthread_mutex_t *mutex){
  if( mutex->lockset == 1 && mutex->owner == active_thread->g_id ){
    sigprocmask(SIG_BLOCK, &signal_mask, NULL);
    mutex->lockset = 0;
    mutex->owner = -1;
    sigprocmask(SIG_UNBLOCK, &signal_mask, NULL);
    return 0;
  }
  return -1;
}

/*
  The gtthread_mutex_destroy() function is analogous to
  pthread_mutex_destroy and frees any resourcs associated with the mutex.
*/
int gtthread_mutex_destroy(gtthread_mutex_t *mutex){
    sigprocmask(SIG_BLOCK, &signal_mask, NULL);
    free(mutex);
    sigprocmask(SIG_UNBLOCK, &signal_mask, NULL);
    return 0;
}
