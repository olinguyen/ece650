#include <stdio.h>
#include <stdlib.h>
#include <unistd.h>
#include <stdbool.h>

#include <sys/time.h>
#include <sys/ipc.h>
#include <sys/msg.h>
#include <sys/types.h>

#include "types.h"
#include "consume_produce.h"
#include "random_distribution.h"

#define MSQID 1337
#define DEBUG 0
struct timeval transmit_start, transmit_end, \
               producer_block_start, producer_block_end;
double producer_block_time = 0.0;

int main(int argc, char** argv) {
#if DEBUG
  printf("Producer process started!\n");
#endif
  int b = strtol(argv[1], NULL, 10); 
  double pt = atof(argv[2]);
  double rs = atof(argv[3]);

  srandom(time(NULL));
  int n = 1000;
  int producer_blocked = 0;
  double std = 1.0;

  int status;
  int id = 0;

  int msqid, msqid_info;
  int msgflg = IPC_CREAT | 0660;
  key_t main_key = MSQID, info_key = MSQID + 1;
  message_buf sbuf = {
    .mtype = 1,
    .remaining = n,
    .is_buffer_full = false
  };

  message_buf infobuf = {
    .mtype = 1,
    .requests_completed = 0,
    .consumer_blocked = 0,
    .consumer_block_time = 0.0,
    .is_buffer_full = false
  };

  message_buf rbuf;
  size_t buf_length;

  if ((msqid = msgget(main_key, msgflg )) < 0) {
    perror("msgget");
    exit(1);
  }

  if ((msqid_info = msgget(info_key, msgflg)) < 0) {
    perror("msgget");
    exit(1);
  }

  time_t previous_time = 0;
  gettimeofday(&transmit_start, NULL);
  while (1) {
    gettimeofday(&transmit_end, NULL);
    time_t elapsed = transmit_end.tv_sec - transmit_start.tv_sec;
    if (previous_time != elapsed && elapsed % LOG_TIME == 0) {
      infobuf.producer_block_time = producer_block_time;
      infobuf.producer_blocked = producer_blocked;
      size_t buf_length = sizeof(infobuf) - sizeof(long);
      if (msgsnd(msqid_info, &infobuf, buf_length, IPC_NOWAIT) < 0) {
        perror("msgsnd (producer)");
        exit(1);
      } else {
      }

      previous_time = elapsed;
#if DEBUG
      printf("producer_blocked %d times\n", producer_blocked);
      printf("Producers block time %.8f\n", producer_block_time);
#endif
      producer_block_time = 0.0;
      producer_blocked = 0;
    }
    double request_wait_time = normal_distribution(pt * 10000, std) / 10000.0;

    sleep(request_wait_time);
    // size of request to be transmitted (cannot exceed buffer size)
    int request_size = (int) normal_distribution(rs, std * 4);
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
    //--sbuf.remaining;

    //buf_length = sizeof(sbuf) - sizeof(long);
    buf_length = request_size;

    if (msgsnd(msqid, &sbuf, buf_length, IPC_NOWAIT) < 0) {
      perror("msgsnd (producer)");
      exit(1);
    } else {
#if DEBUG
      printf("...[%d] producer sent %d, %c of size %d\n", sbuf.remaining, sbuf.item, sbuf.buffer[0], request_size);
#endif
    }
  }

  printf("Producer blocked %d times for %.5f seconds\n", \
      producer_blocked, producer_block_time);

  return 0;
}
