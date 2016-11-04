CC = g++
LIBS = -lm -lpthread
INCPATH = ./include
CFLAGS = -g -Wall -I$(INCPATH) #-std=c99

OBJ = roadmap.o vertex.o

%.o: src/%.cpp
	$(CC) -c -o $@ $< $(CFLAGS)

all: main roadmap vertex

main: main.o vertex.o roadmap.o
	$(CC) -o $@ $^ $(CFLAGS) $(LIBS)

roadmap: roadmap.o
	$(CC) -o $@ $^ $(CFLAGS) $(LIBS)

vertex: vertex.o
	$(CC) -o $@ $^ $(CFLAGS) $(LIBS)

.PHONY: clean

clean:
	-rm -f *.o
	rm -rf ./bin/
