CC = gcc
CFLAGS += -g -Wall

all: libco.a test

libco.a: coroutine.o co_swap.o
	ar cr $@ $^

test: test.o libco.a

coroutine.o: coroutine.c coroutine.h
co_swap.o: co_swap.S
test.o: test.c coroutine.h

clean:
	rm -f *.o *.a test

.PHONY: clean
