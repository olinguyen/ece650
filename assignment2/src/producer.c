#include <stdio.h>
#include <stdlib.h>
#include <unistd.h>
#include <sys/time.h>
#include <sys/types.h>
#include <stdbool.h>

#include <pthread.h>
#include <sys/ipc.h>
#include <sys/msg.h>
#include <sys/wait.h>

#include "random_distribution.h"

#define MAX_BUFFER_SIZE 256
#define MAX_CONSUMERS 256
#define MAX_PRODUCERS 256
#define MSQID 1337
#define DEBUG 0
#define VERBOSE 0

#define STDEV 1.0

/*
TODO:
- message passing
- random distributions (use poisson, or get normal distribution values > 0)
*/

struct timeval transmit_start, transmit_end, \
							 producer_block_start, producer_block_end, \
							 consumer_block_start, consumer_block_end;


double producer_block_time = 0.0, consumer_block_time = 0.0;

int t = 5; // total time of execution
int b = 128; // buffer size
int p = 1; // number of producers
int c = 1; // number of consumers

float pt = 0.0005; // prob. dist. for the random time Pt that the producers must wait between request productions
float rs = 50.0; // prob. dist. of the request size
float ct1 = 0.05; // prob. dist. for the random time Ct1 that the consumers take with probability pi
float ct2 = 0.005; // prob. dist. for the random time Ct2 that the consumers take with probability 1-pi
float pi = 0.5; // probability pi

int requests_completed = 0;
int producer_blocked = 0;
int consumer_blocked = 0;
int count = 0;

void producer_consumer_thread(int num_producers, int num_consumers, int b);
void* ConsumerThread(void *a);
void* ProducerThread(void *a);

void producer_consumer_process(int n, int b);

void produce(float ps, float rs);
void consume(float ct1, float ct2, float pi);

typedef struct {
  int request_size;
  int value;
  bool processed;
} request_t;

request_t buffer[MAX_BUFFER_SIZE];

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

} SharedMemory;


int main(int argc, char** argv) {
  srandom(time(NULL));
  if (argc < 3) {
    printf("Invalid number of arguments.\n");
  } else {
    /*
    t = strtol(argv[1], NULL, 10); // total time of execution
    b = strtol(argv[2], NULL, 10); // buffer size
    p = strtol(argv[3], NULL, 10); // number of producers
    c = strtol(argv[4], NULL, 10); // number of consumers
    pt = atof(argv[5]) ;// prob. dist. for the random time Pt that the producers must wait between request productions
    rs = atof(argv[6]); // prob. dist. of the request size
    ct1 = atof(argv[7]); // prob. dist. for the random time Ct1 that the consumers take with probability pi
    ct2 = atof(argv[8]); // prob. dist. for the random time Ct2 that the consumers take with probability 1-pi
    pi = atof(argv[9]); // probability pi
    */

    struct timeval program_start, program_end;
    gettimeofday(&program_start, NULL);

    producer_consumer_thread(p, c, b);
    //producer_consumer_process(n, b);

    gettimeofday(&program_end, NULL);

    float total_time = ((program_end.tv_sec + program_end.tv_usec / 1000000.0) \
        - (program_start.tv_sec + program_start.tv_usec / 1000000.0));
    float transmit_time = ((transmit_end.tv_sec + transmit_end.tv_usec / 1000000.0) \
        - (transmit_start.tv_sec + transmit_start.tv_usec / 1000000.0));
    //printf("%d,%d,%.6f,%.6f,%.6f\n", \
      n, b, total_time, transmit_time, total_time - transmit_time);
  }

  return 0;
}

void producer_consumer_thread(int num_consumers, int num_producers, int b) {

  SharedMemory sharedmem = {
    .buffer_count = 0,
    .buffer_size = b,
    .buffer = buffer,
    .lock = PTHREAD_MUTEX_INITIALIZER,
    .produce_cond = PTHREAD_COND_INITIALIZER,
    .consume_cond = PTHREAD_COND_INITIALIZER
  };
  for (int i = 0; i < MAX_BUFFER_SIZE; ++i) {
    buffer[i].processed = true;
  }

	double iteration_period = 1.0;

  pthread_t producers_id[MAX_CONSUMERS], consumers_id[MAX_PRODUCERS];
  for(int i = 0; i < num_producers; ++i) {
    pthread_create(&producers_id[i], NULL, ProducerThread, (void*)&sharedmem);
  }
  for(int i = 0; i < num_consumers; ++i) {
    pthread_create(&consumers_id[i], NULL, ConsumerThread, (void*)&sharedmem);
  }

  printf("b, p, c, pt, rs, ct1, ct2, pi, requests_completed, "
         "producer_blocked, consumer_blocked, producer_block_time, "
         "consumer_block_time\n");

	// periodically print info
	for (int i = 0; i < 10; ++i) {
		sleep(10.0);

    if (VERBOSE) {
      printf("%d, P=%d, C=%d\n", i, num_producers, num_consumers);
      printf("Total requests completed %d\n", requests_completed);
      printf("%d times producer_blocked %.2f%%\n", producer_blocked, (double)producer_blocked / count * 100.0);
      printf("%d times consumer_blocked %.2f%%\n", consumer_blocked, (double)consumer_blocked / count * 100.0);
      printf("Producers block time %.8f\n", producer_block_time);
      printf("Consumers block time %.8f\n", consumer_block_time);
    } else {
      printf("%d,%d,%d,%d,%d,%d,%.8f,%.8f\n", b, p, c, \
              requests_completed, producer_blocked, consumer_blocked, \
              producer_block_time, consumer_block_time);
    }

		requests_completed = 0;
    producer_block_time = 0.0;
    consumer_block_time = 0.0;
    count = producer_blocked = consumer_blocked = 0;
	}
}

void* ProducerThread(void *a)
{
  gettimeofday(&transmit_start, NULL);
  SharedMemory* sharedmem = (SharedMemory*)a;
#if DEBUG
  printf("Producer thread started!\n");
#endif

  while (1) {
    count++;
    double request_wait_time = normal_distribution(pt * 10000, STDEV) / 10000.0;
#if DEBUG
    //printf("[producer]...waiting %.4f before next request\n", request_wait_time);
#endif
    sleep(request_wait_time);

    bool is_blocked = false;
    gettimeofday(&producer_block_start, NULL);

    // size of request to be transmitted (cannot exceed buffer size)
    int request_size = (int) normal_distribution(rs, STDEV * 5);

    // Block if buffer is full
    pthread_mutex_lock(&sharedmem->lock);
    while(sharedmem->current_size + request_size >= sharedmem->buffer_size) {
      if (!is_blocked) {
        ++producer_blocked;
      }
      is_blocked = true;
      pthread_cond_wait(&sharedmem->produce_cond, &sharedmem->lock);
    }
    gettimeofday(&producer_block_end, NULL);
		if (is_blocked) {
			producer_block_time += ((producer_block_end.tv_sec + producer_block_end.tv_usec / 1000000.0) \
					- (producer_block_start.tv_sec + producer_block_start.tv_usec / 1000000.0));
		}

    int buffer_idx = sharedmem->buffer_count % sharedmem->buffer_size;
    // place requests in available spot only
    bool is_processed = sharedmem->buffer[buffer_idx].processed;
    while (!is_processed) {
      buffer_idx = (buffer_idx + 1) % sharedmem->buffer_size;
      is_processed = sharedmem->buffer[buffer_idx].processed;
    }
    sharedmem->buffer[buffer_idx].value = rand();
    sharedmem->buffer[buffer_idx].processed = false;
#if DEBUG
    //printf("...%d Produced:%d\n", sharedmem->buffer_count, \
        sharedmem->buffer[sharedmem->buffer_count % sharedmem->buffer_size]);
#endif


    sharedmem->buffer[buffer_idx].request_size = request_size;
    sharedmem->current_size += request_size;
#if DEBUG
    printf("[producer]...generating request size %d\n", request_size);
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
#if DEBUG
  printf("Consumer thread started!\n");
#endif
  while(1) {
    bool is_blocked = false;
    gettimeofday(&consumer_block_start, NULL);
    // block if everything was consumed
    pthread_mutex_lock(&sharedmem->lock);
    while(sharedmem->current_size == 0) {
      if (!is_blocked) {
        ++consumer_blocked;
      }
      is_blocked = true;
      pthread_cond_wait(&sharedmem->consume_cond, &sharedmem->lock);
    }
    gettimeofday(&consumer_block_end, NULL);

		if (is_blocked) {
			consumer_block_time += ((consumer_block_end.tv_sec + consumer_block_end.tv_usec / 1000000.0) \
					- (consumer_block_start.tv_sec + consumer_block_start.tv_usec / 1000000.0));
		}

    int buffer_idx = sharedmem->buffer_count % sharedmem->buffer_size;
    // get a request that was not processed yet
    bool is_processed = sharedmem->buffer[buffer_idx].processed;
    while (is_processed) {
			//printf("%d looking for unprocessed request\n", buffer_idx);
      buffer_idx = (buffer_idx + 1) % sharedmem->buffer_size;
      is_processed = sharedmem->buffer[buffer_idx].processed;
    }

		consume(ct1, ct2, pi);
    int consumed = sharedmem->buffer[buffer_idx].value;
    sharedmem->buffer[buffer_idx].processed = true;
    sharedmem->current_size -= sharedmem->buffer[buffer_idx].request_size;
#if DEBUG
  //  printf("...%d Consumed: %d\n", sharedmem->buffer_count, consumed);
#endif

    if (pi <= (double)rand() / RAND_MAX) {
#if DEBUG
//      printf("[consumer]...waiting ct1 = %.4f before next request\n", ct1);
#endif
    //  sleep(ct1);
    } else {
#if DEBUG
//      printf("[consumer]...waiting ct2 = %.4f before next request\n", ct2);
#endif
     // sleep(ct1);
    }

    --sharedmem->buffer_count;
    ++requests_completed;

    pthread_cond_signal(&sharedmem->produce_cond);
    pthread_mutex_unlock(&sharedmem->lock);
  }
}

void producer_consumer_process(int n, int b)
{
  int pid = fork();

  if (pid == 0) {
    char *argv[3] = {"Command-line", ".", NULL};

#if DEBUG
    printf("Consumer process started!\n");
#endif
    execvp("./consumer", argv);
  } else {
    gettimeofday(&transmit_start, NULL);
#if DEBUG
    printf("Producer process started!\n");
#endif
    int status;

    int msqid, msqid_consumer;
    int msgflg = IPC_CREAT | 0660;
    key_t producer_key = MSQID, consumer_key = MSQID + 1;
    message_buf sbuf = {
      .mtype = 1,
      .remaining = n,
      .is_buffer_full = false
    };

    message_buf rbuf;
    size_t buf_length;

    if ((msqid = msgget(producer_key, msgflg )) < 0) {
      perror("msgget");
      exit(1);
    }

    if ((msqid_consumer = msgget(consumer_key, msgflg)) < 0) {
      perror("msgget");
      exit(1);
    }

    while (sbuf.remaining > 0) {

      // check if buffer is full
      struct msqid_ds buffer_status;
      if (msgctl(msqid, IPC_STAT, &buffer_status)) {
          perror("msgctl");
          exit(1);
      }

      if (buffer_status.msg_qnum >= b) {
#if DEBUG
        printf("Messages on queue: %d\n", (int)buffer_status.msg_qnum);
#endif
        sbuf.is_buffer_full = true;
        // wait for notification that an item was consumed
        // this is blocking
        if (msgrcv(msqid_consumer, &rbuf, sizeof(rbuf) - sizeof(long), 1, 0) < 0) {
          perror("msgrcv (producer)");
          exit(1);
        }
      } else {
        sbuf.is_buffer_full = false;
      }

      // generate random numbers
      sbuf.item = rand() % 10;

      --sbuf.remaining;

      buf_length = sizeof(sbuf) - sizeof(long);

      if (msgsnd(msqid, &sbuf, buf_length, IPC_NOWAIT) < 0) {
        perror("msgsnd (producer)");
        exit(1);
      } else {
#if DEBUG
        printf("...[%d] producer sent %d\n", sbuf.remaining, sbuf.item);
#endif
      }
    }

    // wait for child process to return
    wait(&status);

    gettimeofday(&transmit_end, NULL);
  }
}

void produce(float pt, float rs) {
  int low = 10, high = 100;

  float request_wait_time = rand() % 100 * pt;
  printf("...waiting %.4f before next request\n", request_wait_time);
  sleep(request_wait_time);

  int request_size = low + rand() % (high - low);
  printf("...generating request size %d\n", request_size);

}

void consume(float ct1, float ct2, float pi) {
	double delay;
  if (pi <= (double)rand() / RAND_MAX) {
		// simulates I/O delay (longer)
    delay = normal_distribution(ct1 * 100, STDEV) / 100.0;
  } else {
    delay = normal_distribution(ct2 * 1000, STDEV) / 1000.0;
  }
	sleep(delay);
#if DEBUG
    printf("[consumer]...waiting %.4f before next request\n", delay);
#endif
}
