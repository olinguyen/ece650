CC = gcc
LIBS = -lm -lpthread
INCPATH = ./include
CFLAGs = -g -Wall -I$(INCPATH)

OBJ = consumer.o producer.o

%.o: %.c
	$(CC) -c -o $@ $< $(CFLAGS)

all: consumer producer

consumer: consumer.o
	$(CC) -o $@ $^ $(CFLAGS) $(LIBS)

producer: producer.o
	$(CC) -o $@ $^ $(CFLAGS) $(LIBS)

send: message_send.o
	$(CC) -o $@ $^ $(CFLAGS) $(LIBS)

receive: message_rec.o
	$(CC) -o $@ $^ $(CFLAGS) $(LIBS)

.PHONY: clean

clean:
	-rm -f *.o
	rm -rf ./bin/

