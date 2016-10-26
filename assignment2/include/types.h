#ifndef TYPES_H 
#define TYPES_H

typedef struct {
  int request_size;
  int value;
  bool processed;
} request_t;

typedef struct msgbuf {
    long    mtype;
    int     remaining;
    int     item;
    bool    is_buffer_full;
} message_buf;

typedef struct {
  request_t* buffer; // the buffer itself
  int buffer_size; // max buffer size
  int buffer_count; // number of elements currently in the buffer
  int current_size; // number of bytes in the buffer

  pthread_mutex_t lock;
  pthread_cond_t produce_cond;
  pthread_cond_t consume_cond;

  int id;

} SharedMemory;

#endif
