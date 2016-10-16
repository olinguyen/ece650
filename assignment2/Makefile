CC = gcc
LIBS = -lm -lpthread
INCPATH = ./include
CFLAGs = -g -Wall -I$(INCPATH)

OBJ = consumer.o producer.o test.o

%.o: src/%.c
	$(CC) -c -o $@ $< $(CFLAGS)

all: consumer producer

consumer: consumer.o
	$(CC) -o $@ $^ $(CFLAGS) $(LIBS)

producer: producer.o
	$(CC) -o $@ $^ $(CFLAGS) $(LIBS)

test: test.o
	$(CC) -o $@ $^ $(CFLAGS) $(LIBS)

.PHONY: clean

clean:
	-rm -f *.o
	rm -rf ./bin/

