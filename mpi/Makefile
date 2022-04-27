C_FILES = $(wildcard src/*.c)
O_FILES = $(C_FILES:src/%.c=build/%.o)
CC = mpicc
CFLAGS  = -W -Wall -Wextra  -O3

CC_VERBOSE = $(CC)
CC_NO_VERBOSE = @echo "Building $@..."; $(CC)

ifeq ($(VERBOSE),YES)
  V_CC = $(CC_VERBOSE)
  AT := 
else
  V_CC = $(CC_NO_VERBOSE)
  AT := @
endif

.PHONY: all clean
.DEFAULT: all

all: kn

run:
	./kn < testes/knapsack.in

run1:
	mpirun --np 1 kn < testes/large_scale/knapPI_2_10000_1000_1

run2:
	mpirun --np 2 kn < testes/large_scale/knapPI_2_10000_1000_1

run3:
	mpirun --np 3 kn < testes/large_scale/knapPI_2_10000_1000_1

run4:
	mpirun --np 4 kn < testes/large_scale/knapPI_2_10000_1000_1

teste: ../build/teste.o ../build/grafo.o
	$(CC) $(CFLAGS) -o $@ $^

kn: $(O_FILES)
	$(V_CC) $(CFLAGS) -o $@ $^

docs:
	doxygen configDoxygen

build:
	@mkdir -p build

build/%.o: src/%.c | build
	$(CC) $(CFLAGS) -c $< -o $@

clean:
	@echo Removing object files
	$(AT)-rm -f $(O_FILES)
	@echo Removing application
	$(AT)-rm -f kn
	@echo Removing build directory
	$(AT)-rm -rf build
	# @echo Removing doxygen directory
	# $(AT)-rm -rf doxygen
