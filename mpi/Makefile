C_FILES = $(wildcard src/*.c)
O_FILES = $(C_FILES:src/%.c=build/%.o)
CC = mpicc
CFLAGS  = -std=c99 \
	  -pipe \
	  -ggdb3 -Wstrict-overflow=5 -fstack-protector-all \
          -W -Wall -Wextra \
	  -Wbad-function-cast \
	  -Wcast-align \
	  -Wcast-qual \
	  -Wconversion \
	  -Wfloat-equal \
	  -Wformat-y2k \
	  -Winit-self \
	  -Winline \
	  -Winvalid-pch \
	  -Wmissing-declarations \
	  -Wmissing-field-initializers \
	  -Wmissing-format-attribute \
	  -Wmissing-include-dirs \
	  -Wmissing-noreturn \
	  -Wmissing-prototypes \
	  -Wnested-externs \
	  -Wnormalized=nfc \
	  -Wold-style-definition \
	  -Woverlength-strings \
	  -Wpacked \
	  -Wpadded \
	  -Wpointer-arith \
	  -Wredundant-decls \
	  -Wshadow \
	  -Wsign-compare \
	  -Wstack-protector \
	  -Wstrict-aliasing=2 \
	  -Wstrict-prototypes \
	  -Wundef \
	  -Wunsafe-loop-optimizations \
	  -Wvolatile-register-var \
	  -Wwrite-strings

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

run2:
	# mpirun --hostfile host_file --np 2 kn < testes/knapsack.in
	clear
	mpirun --np 2 kn < testes/knapsack.in

teste: ../build/teste.o ../build/grafo.o
	$(CC) $(CFLAGS) -o $@ $^

kn: $(O_FILES)
	$(V_CC) -o $@ $^

docs:
	doxygen configDoxygen

build:
	@mkdir -p build

build/%.o: src/%.c | build
	$(CC) -c $< -o $@

clean:
	@echo Removing object files
	$(AT)-rm -f $(O_FILES)
	@echo Removing application
	$(AT)-rm -f kn
	@echo Removing build directory
	$(AT)-rm -rf build
	# @echo Removing doxygen directory
	# $(AT)-rm -rf doxygen
