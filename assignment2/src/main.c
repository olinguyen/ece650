#include <stdio.h>
#include <stdlib.h>
#include <unistd.h>
#include <stdbool.h>
#include <pthread.h>
#include <signal.h>

#include <sys/time.h>
#include <sys/types.h>
#include <sys/ipc.h>
#include <sys/msg.h>
#include <sys/wait.h>

#include "random_distribution.h"
#include "consume_produce.h"
#include "types.h"

#define MAX_BUFFER_SIZE 256
#define MAX_CONSUMERS 256
#define MAX_PRODUCERS 256
#define MSQID 1337
#define DEBUG 0
#define VERBOSE 0

#define STDEV 1.0

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

float pt = 0.0005; // prob. dist. for the random time Pt that the producers must wait between request productions
float rs = 50.0; // prob. dist. of the request size
float ct1 = 0.05; // prob. dist. for the random time Ct1 that the consumers take with probability pi
float ct2 = 0.005; // prob. dist. for the random time Ct2 that the consumers take with probability 1-pi
float pi = 0.5; // probability pi

request_t buffer[MAX_BUFFER_SIZE];
int requests_completed = 0;
int producer_blocked = 0;
int consumer_blocked = 0;
int count = 0;

void producer_consumer_thread(int num_producers, int num_consumers, int b);
void* ConsumerThread(void *a);
void* ProducerThread(void *a);
void ConsumerProcess();
void ProducerProcess();

void producer_consumer_process(int n, int b);


int main(int argc, char** argv) {
  srandom(time(NULL));
  if (argc < 3) {
    printf("Invalid number of arguments.\n");
  } else {

    t = strtol(argv[1], NULL, 10); // total time of execution
    b = strtol(argv[2], NULL, 10); // buffer size
    p = strtol(argv[3], NULL, 10); // number of producers
    c = strtol(argv[4], NULL, 10); // number of consumers
    pt = atof(argv[5]) ;// prob. dist. for the random time Pt that the producers must wait between request productions
    rs = atof(argv[6]); // prob. dist. of the request size
    ct1 = atof(argv[7]); // prob. dist. for the random time Ct1 that the consumers take with probability pi
    ct2 = atof(argv[8]); // prob. dist. for the random time Ct2 that the consumers take with probability 1-pi
    pi = atof(argv[9]); // probability pi

    struct timeval program_start, program_end;
    gettimeofday(&program_start, NULL);

    //producer_consumer_thread(p, c, b);
    int n = 32;
    producer_consumer_process(n, b);

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
  int i;

  SharedMemory sharedmem = {
    .buffer_count = 0,
    .buffer_size = b,
    .buffer = buffer,
    .lock = PTHREAD_MUTEX_INITIALIZER,
    .produce_cond = PTHREAD_COND_INITIALIZER,
    .consume_cond = PTHREAD_COND_INITIALIZER
  };
  for (i = 0; i < MAX_BUFFER_SIZE; ++i) {
    buffer[i].processed = true;
  }

  pthread_t producers_id[MAX_CONSUMERS], consumers_id[MAX_PRODUCERS];
  for(i = 0; i < num_producers; ++i) {
    sharedmem.id = i;
    pthread_create(&producers_id[i], NULL, ProducerThread, (void*)&sharedmem);
  }
  for(i = 0; i < num_consumers; ++i) {
    sharedmem.id = i;
    pthread_create(&consumers_id[i], NULL, ConsumerThread, (void*)&sharedmem);
  }

  /*
  printf("b,p,c,pt,rs,ct1,ct2,pi,requests_completed,"
         "producer_blocked,consumer_blocked,producer_block_time,"
         "consumer_block_time\n");
  */
  int num_iterations = t / 10;

	// periodically print info
	for (i = 0; i < num_iterations; ++i) {
		sleep(10.0);

    if (VERBOSE) {
      printf("%d, P=%d, C=%d\n", i, num_producers, num_consumers);
      printf("Total requests completed %d\n", requests_completed);
      printf("%d times producer_blocked %.2f%%\n", producer_blocked, (double)producer_blocked / requests_completed * 100.0);
      printf("%d times consumer_blocked %.2f%%\n", consumer_blocked, (double)consumer_blocked / requests_completed * 100.0);
      printf("Producers block time %.8f\n", producer_block_time);
      printf("Consumers block time %.8f\n", consumer_block_time);
    } else {
      printf("%d,%d,%d,%.5f,%.2f,%.4f,%.4f,%.2f,%d,%d,%d,%.8f,%.8f\n", b, p, c, \
              pt, rs, ct1, ct2, pi, \
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
  int id = sharedmem->id;
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
    gettimeofday(&producer_blocks_start[id], NULL);

    // size of request to be transmitted (cannot exceed buffer size)
    int request_size = (int) normal_distribution(rs, STDEV * 4);

    // Block if buffer is full
    pthread_mutex_lock(&sharedmem->lock);
    while(sharedmem->current_size + request_size >= sharedmem->buffer_size) {
      if (!is_blocked) {
        ++producer_blocked;
      }
      is_blocked = true;
      pthread_cond_wait(&sharedmem->produce_cond, &sharedmem->lock);
    }
    gettimeofday(&producer_blocks_end[id], NULL);
		if (is_blocked) {
			producer_block_time += ((producer_blocks_end[id].tv_sec + producer_blocks_end[id].tv_usec / 1000000.0) \
					- (producer_blocks_start[id].tv_sec + producer_blocks_start[id].tv_usec / 1000000.0));
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
  int id = sharedmem->id;
#if DEBUG
  printf("Consumer thread started!\n");
#endif
  while(1) {
    bool is_blocked = false;
    gettimeofday(&consumer_blocks_start[id], NULL);
    // block if everything was consumed
    pthread_mutex_lock(&sharedmem->lock);
    while(sharedmem->current_size == 0) {
      if (!is_blocked) {
        ++consumer_blocked;
      }
      is_blocked = true;
      pthread_cond_wait(&sharedmem->consume_cond, &sharedmem->lock);
    }
    gettimeofday(&consumer_blocks_end[id], NULL);

		if (is_blocked) {
			consumer_block_time += ((consumer_blocks_end[id].tv_sec + consumer_blocks_end[id].tv_usec / 1000000.0) \
					- (consumer_blocks_start[id].tv_sec + consumer_blocks_start[id].tv_usec / 1000000.0));
		}

    int buffer_idx = sharedmem->buffer_count % sharedmem->buffer_size;
    // get a request that was not processed yet
    bool is_processed = sharedmem->buffer[buffer_idx].processed;
    while (is_processed) {
			//printf("%d looking for unprocessed request\n", buffer_idx);
      buffer_idx = (buffer_idx + 1) % sharedmem->buffer_size;
      is_processed = sharedmem->buffer[buffer_idx].processed;
    }

		consume(ct1, ct2, pi, STDEV);
    int consumed = sharedmem->buffer[buffer_idx].value;
    sharedmem->buffer[buffer_idx].processed = true;
    sharedmem->current_size -= sharedmem->buffer[buffer_idx].request_size;
#if DEBUG
  //  printf("...%d Consumed: %d\n", sharedmem->buffer_count, consumed);
#endif

    --sharedmem->buffer_count;
    ++requests_completed;

    pthread_cond_signal(&sharedmem->produce_cond);
    pthread_mutex_unlock(&sharedmem->lock);
  }
}

void producer_consumer_process(int n, int b)
{
  int i;
  int msqid_info;
  int msgflg = IPC_CREAT | 0660;
  key_t info_key = MSQID + 1;
  message_buf rbuf;

  if ((msqid_info = msgget(info_key, msgflg)) < 0) {
    perror("msgget");
    exit(1);
  }

  pid_t consumer_pids[MAX_CONSUMERS], \
        producer_pids[MAX_PRODUCERS];

  pid_t pid;

  char pts[10], rss[5], ct1s[10], ct2s[10], pis[5], bs[5];
  snprintf(pts, 10, "%f", pt);
  snprintf(rss, 5, "%f", rs);
  snprintf(ct1s, 10, "%f", ct1);
  snprintf(ct2s, 10, "%f", ct2);
  snprintf(pis, 5, "%f", pi);
  snprintf(bs, 5, "%d", b);

  for(i = 0; i < c; ++i) {
    char *argv[5] = {"consumer", ct1s, ct2s, pis, NULL};
    if ((consumer_pids[i] = fork()) < 0) {
      perror("fork consumer");
      abort();
    } else if (consumer_pids[i] == 0) {
      execvp("./consumer", argv);
    }
  }

  for(i = 0; i < p; ++i) {
    char *argv[5] = {"producer", bs, pts, rss, NULL};
    if ((producer_pids[i] = fork()) < 0) {
      perror("fork producer");
      abort();
    } else if (producer_pids[i] == 0) {
      execvp("./producer", argv);
    }
  }


  //ProducerProcess();
  // wait for child process to return
  int status;
  int num_iterations = t / LOG_TIME;
  struct msqid_ds buffer_status;

  for(i = 0; i < num_iterations; ++i) {
    sleep(LOG_TIME);
    if (msgctl(msqid_info, IPC_STAT, &buffer_status)) {
        perror("msgctl");
        exit(1);
    }
    while (buffer_status.msg_qnum > 0) {
      --buffer_status.msg_qnum;
      if (msgrcv(msqid_info, &rbuf, sizeof(rbuf) - sizeof(long), 1, 0) < 0) {
        perror("msgrcv");
        exit(1);
      } else {
        requests_completed += rbuf.requests_completed;
        producer_block_time += rbuf.producer_block_time;
        consumer_block_time += rbuf.consumer_block_time;
        producer_blocked += rbuf.producer_blocked;
        consumer_blocked += rbuf.consumer_blocked;
      }
    }

    if (VERBOSE) {
      printf("P=%d, C=%d\n", p, c);
      printf("Total requests completed %d\n", requests_completed);
      printf("%d times producer_blocked %.2f%%\n", producer_blocked, (double)producer_blocked / requests_completed * 100.0);
      printf("%d times consumer_blocked %.2f%%\n", consumer_blocked, (double)consumer_blocked / requests_completed * 100.0);
      printf("Producers block time %.8f\n", producer_block_time);
      printf("Consumers block time %.8f\n", consumer_block_time);
    } else {
      printf("%d,%d,%d,%.5f,%.2f,%.4f,%.4f,%.2f,%d,%d,%d,%.8f,%.8f\n", b, p, c, \
              pt, rs, ct1, ct2, pi, \
              requests_completed, producer_blocked, consumer_blocked, \
              producer_block_time, consumer_block_time);
    }

		requests_completed = 0;
    producer_block_time = 0.0;
    consumer_block_time = 0.0;
    count = producer_blocked = consumer_blocked = 0;

  }

  // End all child processes
  for(i = 0; i < c; ++i) {
    kill(consumer_pids[i], SIGKILL);
  }

  for(i = 0; i < p; ++i) {
    kill(producer_pids[i], SIGKILL);
  }

  //printf("Producer blocked %d times for %.5f seconds\n", \
    producer_blocked, producer_block_time);

  /*
  if ((pid = fork()) == 0) {

#if DEBUG
    printf("Consumer process started!\n");
#endif
    execvp("./consumer", argv);
  } else {
    ProducerProcess();
    // wait for child process to return
    int status;
    wait(&status);
  }
  printf("Producer blocked %d times for %.5f seconds\n", \
      producer_blocked, producer_block_time);
  */
}


void ProducerProcess() {
  int n = 1000;
  //execvp("./producer", argv);
  gettimeofday(&transmit_start, NULL);
#if DEBUG
  printf("Producer process started!\n");
#endif
  int id = 0;

  int msqid, msqid_info;
  int msgflg = IPC_CREAT | 0660;
  key_t producer_key = MSQID, info_key = MSQID + 1;
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

  if ((msqid_info = msgget(info_key, msgflg)) < 0) {
    perror("msgget");
    exit(1);
  }

  while (sbuf.remaining > 0) {
    double request_wait_time = normal_distribution(pt * 10000, STDEV) / 10000.0;
    sleep(request_wait_time);
    // size of request to be transmitted (cannot exceed buffer size)
    int request_size = (int) normal_distribution(rs, STDEV * 4);
    bool is_blocked = false;
    struct msqid_ds buffer_status;

    gettimeofday(&producer_block_start, NULL);
    // block if buffer is full
    do {
      if (msgctl(msqid, IPC_STAT, &buffer_status)) {
          perror("msgctl");
          exit(1);
      }
      if (buffer_status.msg_cbytes + request_size >= b && !is_blocked) {
        is_blocked = true;
        ++producer_blocked;
#if DEBUG
        //printf("producer blocked %d times! Messages on queue: %d, bytes on queue: %d\n", producer_blocked, buffer_status.msg_qnum, buffer_status.msg_cbytes);
#endif
      }
    } while (buffer_status.msg_cbytes + request_size >= b);

    gettimeofday(&producer_block_end, NULL);
    if (is_blocked) {
      producer_block_time += ((producer_block_end.tv_sec + producer_block_end.tv_usec / 1000000.0) \
          - (producer_block_start.tv_sec + producer_block_start.tv_usec / 1000000.0));
    }

    // generate requests
    --sbuf.remaining;

    //buf_length = sizeof(sbuf) - sizeof(long);
    buf_length = request_size;

    if (msgsnd(msqid, &sbuf, buf_length, IPC_NOWAIT) < 0) {
      perror("msgsnd (producer)");
      exit(1);
    } else {
#if DEBUG
      //printf("...[%d] producer sent %d, %c\n", sbuf.remaining, sbuf.item, sbuf.buffer[0]);
#endif
    }
  }

  gettimeofday(&transmit_end, NULL);
}
