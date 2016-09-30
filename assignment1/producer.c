#include <stdio.h>
#include <stdlib.h>
#include <unistd.h>
#include <sys/time.h>

#include <pthread.h>
#include <sys/ipc.h>
#include <sys/msg.h>

#define MAX_BUFFER_SIZE 1024
int buffer[MAX_BUFFER_SIZE];

void producer_consumer_thread(int n, int b);
void* ConsumerThread(void *a);
void* ProducerThread(void *a);

void producer_consumer_process(int n, int b);

#define MSGSZ     128
typedef struct msgbuf {
    long    mtype;
    char    mtext[MSGSZ];
    int     buffer[MAX_BUFFER_SIZE];
    int     remaining;
    int     produce_count;
} message_buf;

typedef struct
{
	int* buffer;
	int buffer_size;
	int produce_count;
	int consume_count;
	int n;

	long mtype;

} SharedMemory;

int main(int argc, char** argv)
{
	if (argc < 3)
	{
		printf("Invalid number of arguments.\n");
	}
	else
	{
		int n = strtol(argv[1], NULL, 10);
		int b = strtol(argv[2], NULL, 10);

		struct timeval start, end;
		gettimeofday(&start, NULL);

		/* Initialize system */
//		producer_consumer_thread(n, b);
		producer_consumer_process(n, b);

		gettimeofday(&end, NULL);
		printf("Time to initialize system: <%.4f>\n",
					((end.tv_sec + end.tv_usec / 1000000.0)
				- (start.tv_sec + start.tv_usec / 1000000.0)));
	}

  return 0;
}

void producer_consumer_thread(int n, int b)
{
	SharedMemory sharedmem;
	sharedmem.consume_count = 0;
	sharedmem.produce_count = 0;
	sharedmem.buffer_size = b;
	sharedmem.buffer = buffer;
	sharedmem.n = n;

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

	while (sharedmem->n > 0)
	{
		while(sharedmem->produce_count - sharedmem->consume_count == sharedmem->buffer_size);
		sharedmem->buffer[sharedmem->produce_count % sharedmem->buffer_size] = rand() % 10;

		printf("...%d Produced:%d\n", sharedmem->produce_count, sharedmem->buffer[sharedmem->produce_count % sharedmem->buffer_size]);

		++sharedmem->produce_count;
		--sharedmem->n;
	}
}

void* ConsumerThread(void *a)
{
	SharedMemory* sharedmem = (SharedMemory*)a;
	printf("Consumer thread started!\n");

	while(sharedmem->n > 0)
	{
		while(sharedmem->produce_count - sharedmem->consume_count == 0);

		int consumed = sharedmem->buffer[sharedmem->consume_count % sharedmem->buffer_size];
		printf("...%d Consumed: %d\n", sharedmem->consume_count, consumed);

		++sharedmem->consume_count;
	}
}

void producer_consumer_process(int n, int b)
{
	int pid = fork();

	if (pid == 0)
	{
    char *argv[3] = {"Command-line", ".", NULL};

		printf("Consumer process started!\n");
    execvp("./consumer", argv);
	}
	else
	{
		printf("Producer process started!\n");
    int msqid;
    int msgflg = IPC_CREAT | 0666;
    key_t producer_key;
    message_buf sbuf;
    size_t buf_length;

    producer_key = 1200;

    if ((msqid = msgget(producer_key, msgflg )) < 0) {
        perror("msgget");
        exit(1);
    }

    sbuf.mtype = 1;
    sbuf.remaining = n;
    sbuf.buffer[0] = 99;

    (void) strcpy(sbuf.mtext, "Did you get this?");
    buf_length = sizeof(sbuf) - sizeof(long);

    /*
     * Send a message.
     */
    if (msgsnd(msqid, &sbuf, buf_length, IPC_NOWAIT) < 0) {
        perror("msgsnd");
        exit(1);
    }
    else
      printf("Message: \"%d\" Sent\n", sbuf.buffer[0]);

    printf("...producer receiving from consumer\n");
    if (msgrcv(msqid, &sbuf, sizeof(sbuf), 1, 0) < 0) {
      perror("msgrcv");
      exit(1);
    }
    printf("Producer received %d\n", sbuf.remaining);
	}
}
