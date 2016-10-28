CC = gcc
LIBS = -lm -lpthread
INCPATH = ./include
CFLAGS = -g -Wall -I$(INCPATH) #-std=c99

OBJ = consumer.o producer.o test.o

%.o: src/%.c
	$(CC) -c -o $@ $< $(CFLAGS)

all: main consumer producer 

consumer: consumer.o consume_produce.o random_distribution.o
	$(CC) -o $@ $^ $(CFLAGS) $(LIBS)

producer: producer.o random_distribution.o consume_produce.o
	$(CC) -o $@ $^ $(CFLAGS) $(LIBS)

main: main.o random_distribution.o consume_produce.o
	$(CC) -o $@ $^ $(CFLAGS) $(LIBS)

test: test.o random_distribution.o
	$(CC) -o $@ $^ $(CFLAGS) $(LIBS)

.PHONY: clean

clean:
	-rm -f *.o
	rm -rf ./bin/
