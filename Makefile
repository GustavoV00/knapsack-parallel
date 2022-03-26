CC=gcc
CFLAGS=-Wall -fopenmp
# DEPS = hellomake.h

# %.o: %.c $(DEPS)

all: serial parallel

run1:
	./knapsack-serial < knapsack.in

run2:
	./knapsack-parallel < knapsack.in

%.o: %.c
	$(CC) -c -o $@ $< $(CFLAGS) 

serial: knapsack-serial.o 
	$(CC) -o knapsack-serial knapsack-serial.o $(CFLAGS)

parallel: knapsack-parallel.o
	$(CC) -o knapsack-parallel knapsack-parallel.o $(CFLAGS)


clean:
	rm *.o
	rm knapsack-serial
	rm knapsack-parallel
