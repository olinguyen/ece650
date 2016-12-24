#ifndef TYPES_H
#define TYPES_H
#include <pthread.h>
#include <queue>
#include "Vertex.h"
#include "Edge.h"

#define LOG_TIME 5

typedef struct {
  int request_size;
  int value;
  bool processed;
} request_t;

typedef enum {
	GRAPH = 0,
	PLAN = 1,
	INVALID = 2
} cmd_type;

typedef enum {
  ADD_VERTEX = 0,
  ADD_EDGE = 1,
  EDGE_EVENT = 2,
  ROAD = 3,
  TRIP = 4,
  VERTEX = 5,
  STORE = 6,
  RETRIEVE = 7
} command_e;

typedef struct {
	cmd_type type;
  command_e cmd;
  PointOfInterest poi;
  string v_dst, v_src;
  string e_dst, e_src;
  bool direction;
  double speed;
  double length;
  string label;
  vertex_type vtype;
} command_t;

typedef struct {
  request_t* buffer; // the buffer itself
  int buffer_size; // max buffer size
  int buffer_count; // number of elements currently in the buffer
  int current_size; // number of bytes in the buffer

  pthread_mutex_t lock;
  pthread_cond_t produce_cond;
  pthread_cond_t consume_cond;

  int id;
	std::queue<command_t> command_queue;
	std::queue<string> tcp_queue;

} SharedMemory;

#endif
