  CC = gcc            # default is CC = cc
CFLAGS = -g -Wall   # default is CFLAGS = [blank]

GTTHREADS_SRC = gtthread_sched.c gtthread_mutex.c steque.c
GTTHREADS_OBJ = $(patsubst %.c,%.o,$(GTTHREADS_SRC))

# pattern rule for object files
%.o: %.c
	$(CC) -c $(CFLAGS) $< -o $@

#### GTThreads ####
gtthread_main: gtthread_main.o $(GTTHREADS_OBJ)
	$(CC) -o gtthread_main gtthread_main.o $(GTTHREADS_OBJ)

test1: $(GTTHREADS_OBJ)
	$(CC) -c test1.c
	$(CC) -o test1 test1.o $(GTTHREADS_OBJ)

clean:
	$(RM) -f *.o dining_main gtthread_main
