CFLAGS=-Wall -O3 -I.

all: get_lux

get_lux: main.o max44009.o
	$(CC) $(CFLAGS) -o get_lux main.o max44009.o

main.o: main.c
	$(CC) -c $(CFLAGS) main.c

libmax44009.a: max44009.o
	ar -rc libmax44009.a max44009.o

max44009.o: max44009.c
	$(CC) -c $(CFLAGS) max44009.c

install: libmax44009.a get_lux max44009_test
	cp libmax44009.a /usr/local/lib
	cp max44009.h /usr/local/include
	cp get_lux /usr/local/bin
	cp max44009_test /usr/local/bin

clean:
	rm *.o libmax44009.a
