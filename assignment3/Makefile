CC = g++
LIBS = -lm -lpthread
INCPATH = ./include
CFLAGS = -g -Wall -I$(INCPATH) -Wno-reorder -std=c++11#-std=c99

OBJ = Vertex.o Edge.o Graph.o

%.o: src/%.cpp
	$(CC) -c -o $@ $< $(CFLAGS)

all: main

main: main.o Vertex.o Edge.o Graph.o
	$(CC) -o $@ $^ $(CFLAGS) $(LIBS)

vertex: Vertex.o Edge.o
	$(CC) -o $@ $^ $(CFLAGS) $(LIBS)

edge: Edge.o Vertex.o
	$(CC) -o $@ $^ $(CFLAGS) $(LIBS)

graph: Graph.o Edge.o Vertex.o
	$(CC) -o $@ $^ $(CFLAGS) $(LIBS)

.PHONY: clean

clean:
	-rm -f *.o
	rm -rf ./bin/
