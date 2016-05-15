#include <stdio.h>
#include <stdlib.h>
#include "gtthread.h"

/* Tests creation.
   Should print "Hello World!" */

void *thr1(void *in) {
  printf("Hello World!\n");
  fflush(stdout);
  return NULL;
}

int main() {
  gtthread_t t1;
  // printf(" Reached here\n");
  gtthread_init(0);
  // printf(" Init finished\n");
  gtthread_create( &t1, thr1, NULL);
  // printf("create finish\n");
  gtthread_yield();
  // printf("yield finish\n");
  return EXIT_SUCCESS;
}
