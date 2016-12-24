#include <stdio.h>
#include <stdlib.h>
#include <unistd.h>
#include <stdbool.h>
#include <pthread.h>
#include <signal.h>
#include <semaphore.h>

#include <sys/time.h>
#include <sys/types.h>
#include <sys/ipc.h>
#include <sys/wait.h>
#include <sys/socket.h>
#include <netinet/in.h>
#include <strings.h>

#include "consume_produce.h"
#include "types.h"
#include "Graph.h"

#define MAX_BUFFER_SIZE 256
#define MAX_CONSUMERS 256
#define MAX_PRODUCERS 256
#define DEBUG 0
#define VERBOSE 0
#define LOG 0

#define GRAPH_FILENAME "graph.in"

struct timeval transmit_start, transmit_end, \
							 producer_block_start, producer_block_end, \
							 consumer_block_start, consumer_block_end;

struct timeval producer_blocks_start[MAX_PRODUCERS], producer_blocks_end[MAX_PRODUCERS], \
							 consumer_blocks_start[MAX_CONSUMERS], consumer_blocks_end[MAX_CONSUMERS];

double producer_block_time = 0.0, consumer_block_time = 0.0;

int t = 50; // total time of execution
int b = 128; // buffer size
int p = 1; // number of producers
int c = 1; // number of consumers
int port = 8000; // port number

sem_t empty; // remaining space on buffer
sem_t full; // items produced

request_t buffer[MAX_BUFFER_SIZE];
int requests_completed = 0;
int producer_blocked = 0;
int consumer_blocked = 0;
int count = 0;

Graph g;

void producer_consumer_thread(int num_producers, int num_consumers, int b);
void* ConsumerThread(void *a);
void* ProducerThread(void *a);
void* CommandHandlerThread(void *a);
void ConsumerProcess();
void ProducerProcess();

void process_command(command_t cmd); 
command_t parse_command(string input_string);
vector<string> split(const string& str, const string& delim);
void error(char *msg);

int main(int argc, char** argv) {
  srandom(time(NULL));
  if (argc < 2) {
    printf("Invalid number of arguments.\n");
    printf("Usage: ./main <port> <runtime> <buffer size> <num_producers <num_consumers>\n");
  } else {

		port = atoi(argv[1]);
		/*
    t = strtol(argv[2], NULL, 10); // total time of execution
    b = strtol(argv[3], NULL, 10); // buffer size
    p = strtol(argv[4], NULL, 10); // number of producers
    c = strtol(argv[5], NULL, 10); // number of consumers
		*/

    struct timeval program_start, program_end;
    gettimeofday(&program_start, NULL);

    producer_consumer_thread(p, c, b);

    gettimeofday(&program_end, NULL);

    float total_time = ((program_end.tv_sec + program_end.tv_usec / 1000000.0) \
        - (program_start.tv_sec + program_start.tv_usec / 1000000.0));
  }

  return 0;
}

void producer_consumer_thread(int num_consumers, int num_producers, int b) {
  int i;

  SharedMemory sharedmem;
  sharedmem.buffer_count = 0;
  sharedmem.buffer_size = b;
  sharedmem.buffer = buffer;
  sharedmem.lock = PTHREAD_MUTEX_INITIALIZER;
  sharedmem.produce_cond = PTHREAD_COND_INITIALIZER;
  sharedmem.consume_cond = PTHREAD_COND_INITIALIZER;

  sem_init(&empty, 0, b);
  sem_init(&full, 0, 0);

  for (i = 0; i < MAX_BUFFER_SIZE; ++i) {
    buffer[i].processed = true;
  }

  pthread_t producers_id[MAX_CONSUMERS], consumers_id[MAX_PRODUCERS];
	pthread_t handler_id;

	pthread_create(&handler_id, NULL, CommandHandlerThread, (void*)&sharedmem);

  for(i = 0; i < num_producers; ++i) {
    sharedmem.id = i;
    pthread_create(&producers_id[i], NULL, ProducerThread, (void*)&sharedmem);
  }
  for(i = 0; i < num_consumers; ++i) {
    sharedmem.id = i;
    pthread_create(&consumers_id[i], NULL, ConsumerThread, (void*)&sharedmem);
  }
  int num_iterations = t / LOG_TIME;

	// periodically print info
	for (i = 0; i < num_iterations; ++i) {
		sleep(LOG_TIME);

#if LOG
		printf("%d,%d,%d,%d\n", b, p, c, requests_completed)
#endif

		requests_completed = 0;
    producer_block_time = 0.0;
    consumer_block_time = 0.0;
    count = producer_blocked = consumer_blocked = 0;
	}
}

void* CommandHandlerThread(void *a)
{
	printf("Server started. Listening on port %d...\n", port);
#if DEBUG
	printf("Command handler thread started!\n");
#endif
	int sockfd, newsockfd;
	socklen_t clilen;
	char buffer[256];
	struct sockaddr_in serv_addr, cli_addr;
	int n;

	g.retrieve(GRAPH_FILENAME);

  SharedMemory* sharedmem = (SharedMemory*)a;

	sockfd = socket(AF_INET, SOCK_STREAM, 0);

	if (sockfd < 0) {
		error("ERROR opening socket");
	}

	bzero((char *) &serv_addr, sizeof(serv_addr));

	serv_addr.sin_family = AF_INET;
	serv_addr.sin_addr.s_addr = INADDR_ANY;
	serv_addr.sin_port = htons(port);

	if (bind(sockfd, (struct sockaddr *) &serv_addr, sizeof(serv_addr)) < 0) {
		error("ERROR on binding");
	}

  while (1) {
    listen(sockfd, 5);
    clilen = sizeof(cli_addr);

    newsockfd = accept(sockfd,(struct sockaddr *) &cli_addr, &clilen);

    if (newsockfd < 0) {
      error("ERROR on accept");
    }

    bzero(buffer,256);

    n = read(newsockfd,buffer,255);
    //check(buffer);

    if (n < 0) {
      error("ERROR reading from socket");
    }

		sem_wait(&empty);
    printf("Command received: %s", buffer);
		string input_string(buffer);
		sharedmem->tcp_queue.push(input_string);
		sem_post(&full);

    n = write(newsockfd, "Response: 200 OK", 18);

    if (n < 0) {
      error("ERROR writing to socket");
    }
  }
}

void* ProducerThread(void *a)
{
  static int in = 0;
  gettimeofday(&transmit_start, NULL);
  SharedMemory* sharedmem = (SharedMemory*)a;
  int id = sharedmem->id;
#if DEBUG
  printf("Producer thread started!\n");
#endif

  while (1) {
    // wait for command to arrive
    sem_wait(&full);

		// parse string command, validate
		cout << "Producer validating command..." << endl;
		string current_string = sharedmem->tcp_queue.front();	
		command_t current_command = parse_command(current_string);
		// forward command to consumers
    sem_post(&empty);

    count++;

    bool is_blocked = false;
    gettimeofday(&producer_blocks_start[id], NULL);

    // size of request to be transmitted (cannot exceed buffer size)
    int request_size = 1;

    // Block if buffer is full
    pthread_mutex_lock(&sharedmem->lock);
    while(sharedmem->command_queue.size() > sharedmem->buffer_size) {
      pthread_cond_wait(&sharedmem->produce_cond, &sharedmem->lock);
    }
    gettimeofday(&producer_blocks_end[id], NULL);

    int buffer_idx = sharedmem->buffer_count % sharedmem->buffer_size;
		sharedmem->command_queue.push(current_command);

#if DEBUG
    //printf("...%d Produced:%d\n", sharedmem->buffer_count, \
        sharedmem->buffer[sharedmem->buffer_count % sharedmem->buffer_size]);
#endif

    sharedmem->buffer[buffer_idx].request_size = request_size;
    sharedmem->current_size += request_size;
#if DEBUG
    printf("[producer]...queue status %d/%d bytes\n", sharedmem->current_size, sharedmem->buffer_size);
#endif
    // increment buffer by request size
    ++sharedmem->buffer_count;

    pthread_cond_signal(&sharedmem->consume_cond);
    pthread_mutex_unlock(&sharedmem->lock);
  }
  gettimeofday(&transmit_end, NULL);
}

void* ConsumerThread(void *a)
{
  SharedMemory* sharedmem = (SharedMemory*)a;
  int id = sharedmem->id;
#if DEBUG
  printf("Consumer thread started!\n");
#endif
  while(1) {
    bool is_blocked = false;
    gettimeofday(&consumer_blocks_start[id], NULL);
    // block if everything was consumed
    pthread_mutex_lock(&sharedmem->lock);
    while(sharedmem->command_queue.size() == 0) {
      pthread_cond_wait(&sharedmem->consume_cond, &sharedmem->lock);
    }
    gettimeofday(&consumer_blocks_end[id], NULL);

    int buffer_idx = sharedmem->buffer_count % sharedmem->buffer_size;
    // get a request that was not processed yet

		// pick next command to process from command queue
		cout << "Consumer processing command..." << endl;
		command_t current_command = sharedmem->command_queue.front();
		sharedmem->command_queue.pop();
		process_command(current_command);	

    sharedmem->current_size -= sharedmem->buffer[buffer_idx].request_size;

    --sharedmem->buffer_count;
    ++requests_completed;

    pthread_cond_signal(&sharedmem->produce_cond);
    pthread_mutex_unlock(&sharedmem->lock);
  }
}

void process_command(command_t cmd) {
  command_e command = cmd.cmd;  
	cmd_type type = cmd.type;	
  PointOfInterest poi = cmd.poi;
  Vertex v_src = g.vertex(cmd.v_src);
  Vertex v_dst = g.vertex(cmd.v_dst);
  vertex_type vtype = cmd.vtype;
  string vname = cmd.label;
  bool direction = cmd.direction;
  double speed = cmd.speed;
  double length = cmd.length;
  vector<int> output;

	if (type == INVALID) {
		return;
	}

  switch (command) {
    case ADD_VERTEX:
      g.addVertex(vtype, vname);
      break;
    case ADD_EDGE:
      g.addEdge(v_src, v_dst, direction, speed, length);
      break;
    case TRIP:
      output = g.trip(v_src, v_dst);
      break;
    case VERTEX:
      g.vertex(poi);
      break;
    case STORE:
      g.store(GRAPH_FILENAME);
      break;
    case RETRIEVE:
      g.retrieve(GRAPH_FILENAME);
      break;
    default:
			cout << "Invalid command sent" << endl;
      break;
  }
}

vector<string> split(const string& str, const string& delim) {
	vector<string> tokens;
	size_t prev = 0, pos = 0;
	do {
		pos = str.find(delim, prev);
		if (pos == string::npos) pos = str.length();
		string token = str.substr(prev, pos-prev);
		if (!token.empty()) tokens.push_back(token);
		prev = pos + delim.length();
	}
	while (pos < str.length() && prev < str.length());
	return tokens;
}

command_t parse_command(string input_string) {
	string delimiter = " ";		
	command_t command;
	vector<string> split_string = split(input_string, delimiter);	
	string command_type = split_string[0];
	if (command_type.compare("graph") == 0) {
		command.type = GRAPH;	
		string management_type = split_string[1];
		if (management_type.compare("add_vertex")) {
		} else if (management_type.compare("add_edge")) {
		} else if (management_type.compare("edge_event")) {
		} else {
			command.type = INVALID;
		}
	} else if (command_type.compare("plan") == 0) {
		command.type = PLAN;
		string param1 = split_string[1];
		string param2 = split_string[2];
		command.cmd = TRIP;	
	} else {
		command.type = INVALID;
	}
	return command;
}

void error(char *msg) {
	perror(msg);
	exit(1);
}
