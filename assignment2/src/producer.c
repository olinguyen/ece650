#include <stdlib.h>
#include <stdio.h>
#include <sys/ipc.h>
#include <sys/msg.h>
#include <sys/types.h>
#include <stdbool.h>

#include "types.h"
#include "consume_produce.h"

#define MSQID 1337
#define DEBUG 1

struct timeval producer_block_start, producer_block_end;
double producer_block_time = 0.0;

int main(int argc, char** argv) {
  float pt = 0.0005;
  int n = 32;
  int b = 128;
  int producer_blocked;
  double rs = 50.0;
  double std = 1.0;

#if DEBUG
  printf("Producer process started!\n");
#endif
  int status;
  int id = 0;

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
    double request_wait_time = normal_distribution(pt * 10000, std) / 10000.0;

    printf("sleeping %.5f...\n", request_wait_time);
    //sleep(request_wait_time);
    sleep(0.01);
    printf("gucci\n");
    // size of request to be transmitted (cannot exceed buffer size)
    int request_size = (int) normal_distribution(rs, std * 4);
    bool is_blocked = false;
    struct msqid_ds buffer_status;

    gettimeofday(&producer_block_start, NULL);
    printf("checking buffer...\n");
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
        printf("producer blocked %d times! Messages on queue: %d, bytes on queue: %d\n", producer_blocked, buffer_status.msg_qnum, buffer_status.msg_cbytes);
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
      printf("...[%d] producer sent %d, %c\n", sbuf.remaining, sbuf.item, sbuf.buffer[0]);
#endif
    }
  }

  printf("Producer blocked %d times for %.5f seconds\n", \
      producer_blocked, producer_block_time);

  return 0;
}
