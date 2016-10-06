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
int buffer[MAX_BUFFER_SIZE];

void producer_consumer_thread(int n, int b);
void* ConsumerThread(void *a);
void* ProducerThread(void *a);

void producer_consumer_process(int n, int b);

typedef struct msgbuf {
    long    mtype;
    int     remaining;
    int     item;
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
  if (argc < 3) {
    printf("Invalid number of arguments.\n");
  } else {
    int n = strtol(argv[1], NULL, 10);
    int b = strtol(argv[2], NULL, 10);

    struct timeval start, end;
    gettimeofday(&start, NULL);

    //producer_consumer_thread(n, b);
    producer_consumer_process(n, b);

    gettimeofday(&end, NULL);
    printf("Time to execute: <%.4f>\n",
          ((end.tv_sec + end.tv_usec / 1000000.0)
        - (start.tv_sec + start.tv_usec / 1000000.0)));
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
  SharedMemory* sharedmem = (SharedMemory*)a;
  printf("Producer thread started!\n");

  while (sharedmem->n > 0) {
    // Block if buffer is full
    pthread_mutex_lock(&sharedmem->lock);
    while(sharedmem->buffer_count == sharedmem->buffer_size) {
      pthread_cond_wait(&sharedmem->produce_cond, &sharedmem->lock);
    }

    sharedmem->buffer[sharedmem->buffer_count % sharedmem->buffer_size] = rand() % 10;
    printf("...%d Produced:%d\n", sharedmem->buffer_count, \
        sharedmem->buffer[sharedmem->buffer_count % sharedmem->buffer_size]);
    ++sharedmem->buffer_count;
    --sharedmem->n;

    pthread_cond_signal(&sharedmem->consume_cond);
    pthread_mutex_unlock(&sharedmem->lock);
  }
}

void* ConsumerThread(void *a)
{
  SharedMemory* sharedmem = (SharedMemory*)a;
  printf("Consumer thread started!\n");

  while(sharedmem->n > 0) {
    // block if everything was consumed
    pthread_mutex_lock(&sharedmem->lock);
    while(sharedmem->buffer_count == 0) {
      pthread_cond_wait(&sharedmem->consume_cond, &sharedmem->lock);
    }

    int consumed = sharedmem->buffer[sharedmem->buffer_count % sharedmem->buffer_size];
    printf("...%d Consumed: %d\n", sharedmem->buffer_count, consumed);

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

    printf("Consumer process started!\n");
    execvp("./consumer", argv);
  } else {
    printf("Producer process started!\n");
    int status;

    int msqid, msqid_consumer;
    int msgflg = IPC_CREAT | 0660;
    key_t producer_key;
    message_buf sbuf = {
      .mtype = 1,
      .remaining = n,
    };

    message_buf rbuf;

    size_t buf_length;

    producer_key = MSQID;

    if ((msqid = msgget(producer_key, msgflg )) < 0) {
      perror("msgget");
      exit(1);
    }

    key_t consumer_key = MSQID + 1;
    if ((msqid_consumer = msgget(consumer_key, msgflg)) < 0) {
      perror("msgget");
      exit(1);
    }

    while (sbuf.remaining > 0) {
      // generate random numbers
      sbuf.item = rand() % 10;

      --sbuf.remaining;

      buf_length = sizeof(sbuf) - sizeof(long);

      if (msgsnd(msqid, &sbuf, buf_length, IPC_NOWAIT) < 0) {
        perror("msgsnd (producer)");
        exit(1);
      } else {
        printf("...[%d] producer sent %d\n", sbuf.remaining, sbuf.item);
      }

      struct msqid_ds buffer_status;

      if (msgctl(msqid, IPC_STAT, &buffer_status)) {
          perror("msgctl");
          exit(1);
      }

      if (buffer_status.msg_qnum > b) {
        printf("Messages on queue: %d\n", buffer_status.msg_qnum);
				// wait for notification that things were consumed
				// this is blocking
				if (msgrcv(msqid_consumer, &rbuf, sizeof(rbuf) - sizeof(long), 1, 0) < 0) {
					perror("msgrcv (producer)");
					exit(1);
				}
      }
    }

    // wait for child process to return
    wait(&status);
  }
}
