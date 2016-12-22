CC = g++
LIBS = -lm -lpthread
INCPATH = ./include
CFLAGS = -g -Wall -I$(INCPATH) -Wno-reorder -std=c++11#-std=c99

OBJ = consumer.o producer.o test.o Vertex.o Edge.o Graph.o roadmap.o

%.o: src/%*.cpp*
	$(CC) -c -o $@ $< $(CFLAGS)

all: main 

consumer: consumer.o consume_produce.o random_distribution.o
	$(CC) -o $@ $^ $(CFLAGS) $(LIBS)

producer: producer.o random_distribution.o consume_produce.o
	$(CC) -o $@ $^ $(CFLAGS) $(LIBS)

main: main.o random_distribution.o consume_produce.o
	$(CC) -o $@ $^ $(CFLAGS) $(LIBS)

roadmap: roadmap.o Vertex.o Edge.o Graph.o
	$(CC) -o $@ $^ $(CFLAGS) $(LIBS)

vertex: Vertex.o Edge.o
	$(CC) -o $@ $^ $(CFLAGS) $(LIBS)

edge: Edge.o Vertex.o
	$(CC) -o $@ $^ $(CFLAGS) $(LIBS)

graph: Graph.o Edge.o Vertex.o
	$(CC) -o $@ $^ $(CFLAGS) $(LIBS)

test: test.o random_distribution.o
	$(CC) -o $@ $^ $(CFLAGS) $(LIBS)

.PHONY: clean

clean:
	-rm -f *.o
	rm -rf ./bin/
