# GT-Threads
Implement User Level Threading library with an API similar to Pthreads with preemptive round robin scheduler.
Demonstrate the library by further implementing a solution to the classic Dining Philosophers problem.
Each thread is assigned a time slice (its quantum) for which it is allowed to run; a thread is preempted if it used up its quantum. Preemption can be achieved by using an alarm signal as a timer. 

Any program using this GTThreads package must call the function first.
