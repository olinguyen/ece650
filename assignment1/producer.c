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

#define MAX_BUFFER_SIZE 1024
#define MSQID 1444
#define DEBUG 0

int buffer[MAX_BUFFER_SIZE];
struct timeval transmit_start, transmit_end;

void producer_consumer_thread(int n, int b);
void* ConsumerThread(void *a);
void* ProducerThread(void *a);

void producer_consumer_process(int n, int b);

typedef struct msgbuf {
    long    mtype;
    int     remaining;
    int     item;
    bool    is_buffer_full;
} message_buf;

typedef struct {
  int* buffer; // the buffer itself
  int buffer_size; // max buffer size
  int buffer_count; // number of elements currently in the buffer
  int n; // total elements to be produced

  pthread_mutex_t lock;
  pthread_cond_t produce_cond;
  pthread_cond_t consume_cond;

} SharedMemory;

int main(int argc, char** argv) {
  int N[6] = {20000, 40000, 80000, 160000, 320000, 640000};
  int B[5] = {16, 32, 64, 128, 256};

  printf("n,b,total_time,transmit_time,init_time\n");
  if (argc < 3) {
    printf("Invalid number of arguments.\n");
  } else {
    int n = strtol(argv[1], NULL, 10);
    int b = strtol(argv[2], NULL, 10);

    for(int i = 0; i < 5; ++i) {
      b = B[i];
      for (int j = 0; j < 6; ++j) {
        n = N[j];

        for (int k = 0; k < 100; ++k) {
          struct timeval program_start, program_end;
          gettimeofday(&program_start, NULL);

          //producer_consumer_thread(n, b);
          producer_consumer_process(n, b);

          gettimeofday(&program_end, NULL);

          float total_time = ((program_end.tv_sec + program_end.tv_usec / 1000000.0) \
              - (program_start.tv_sec + program_start.tv_usec / 1000000.0));
          float transmit_time = ((transmit_end.tv_sec + transmit_end.tv_usec / 1000000.0) \
              - (transmit_start.tv_sec + transmit_start.tv_usec / 1000000.0));
          printf("%d,%d,%.6f,%.6f,%.6f\n", \
            n, b, total_time, transmit_time, total_time - transmit_time);

        }
      }
    }

  }

  return 0;
}

void producer_consumer_thread(int n, int b) {

  SharedMemory sharedmem = {
    .buffer_count = 0,
    .buffer_size = b,
    .buffer = buffer,
    .n = n,
    .lock = PTHREAD_MUTEX_INITIALIZER,
    .produce_cond = PTHREAD_COND_INITIALIZER,
    .consume_cond = PTHREAD_COND_INITIALIZER
  };

  pthread_t consumer_id, producer_id;
  pthread_create(&producer_id, NULL, ProducerThread, (void*)&sharedmem);
  pthread_create(&consumer_id, NULL, ConsumerThread, (void*)&sharedmem);

  pthread_join(producer_id, NULL);
  pthread_join(consumer_id, NULL);
}

void* ProducerThread(void *a)
{
  gettimeofday(&transmit_start, NULL);
  SharedMemory* sharedmem = (SharedMemory*)a;
#if DEBUG
  printf("Producer thread started!\n");
#endif

  while (sharedmem->n > 0) {
    // Block if buffer is full
    pthread_mutex_lock(&sharedmem->lock);
    while(sharedmem->buffer_count == sharedmem->buffer_size) {
      pthread_cond_wait(&sharedmem->produce_cond, &sharedmem->lock);
    }

    sharedmem->buffer[sharedmem->buffer_count % sharedmem->buffer_size] = rand() % 10;
#if DEBUG
    printf("...%d Produced:%d\n", sharedmem->buffer_count, \
        sharedmem->buffer[sharedmem->buffer_count % sharedmem->buffer_size]);
#endif
    ++sharedmem->buffer_count;
    --sharedmem->n;

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

  while(sharedmem->n > 0) {
    // block if everything was consumed
    pthread_mutex_lock(&sharedmem->lock);
    while(sharedmem->buffer_count == 0) {
      pthread_cond_wait(&sharedmem->consume_cond, &sharedmem->lock);
    }

    int consumed = sharedmem->buffer[sharedmem->buffer_count % sharedmem->buffer_size];
#if DEBUG
    printf("...%d Consumed: %d\n", sharedmem->buffer_count, consumed);
#endif

    --sharedmem->buffer_count;

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
        printf("Messages on queue: %d\n", buffer_status.msg_qnum);
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
