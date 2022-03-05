CC=gcc
CFLAGS=-Wall
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
	$(CC) -o knapsack-serial knapsack-serial.o

parallel: knapsack-parallel.o
	$(CC) -o knapsack-parallel knapsack-parallel.o


clean:
	rm *.o
