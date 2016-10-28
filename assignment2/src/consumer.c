#include <stdlib.h>
#include <stdio.h>
#include <sys/ipc.h>
#include <sys/msg.h>
#include <sys/types.h>
#include <stdbool.h>

#include "types.h"
#include "consume_produce.h"

#define MSQID 1337
#define DEBUG 0

int requests_completed = 0;
int consumer_blocked = 0;
struct timeval transmit_start, transmit_end, \
							 consumer_block_start, consumer_block_end;
double consumer_block_time = 0.0;

int main(int argc, char** argv) {

  float ct1 = 0.05;
  float ct2 = 0.005;
  float pi = 0.5;
  const float std = 1.0;

  int msqid, msqid_consumer;
  key_t producer_key = MSQID, consumer_key = MSQID + 1;
  int msgflg = IPC_CREAT | 0660;
  message_buf  rbuf;

  message_buf sbuf = {
    .mtype = 1,
    .remaining = 0,
    .is_buffer_full = false
  };

  if ((msqid = msgget(producer_key, msgflg)) < 0) {
    perror("msgget");
    exit(1);
  }

  if ((msqid_consumer = msgget(consumer_key, msgflg)) < 0) {
    perror("msgget");
    exit(1);
  }

  do {
    // check if buffer is empty
    struct msqid_ds buffer_status;
    bool is_blocked = false;
    gettimeofday(&consumer_block_start, NULL);
    do {
      if (msgctl(msqid, IPC_STAT, &buffer_status)) {
          perror("msgctl");
          exit(1);
      }
      if (buffer_status.msg_qnum == 0 && !is_blocked) {
        is_blocked = true;
        ++consumer_blocked;
      }
    } while (buffer_status.msg_qnum == 0);

    gettimeofday(&consumer_block_end, NULL);

		if (is_blocked) {
			consumer_block_time += ((consumer_block_end.tv_sec + consumer_block_end.tv_usec / 1000000.0) \
					- (consumer_block_start.tv_sec + consumer_block_start.tv_usec / 1000000.0));
		}

    // get message from producer
    // this is blocking
    if (msgrcv(msqid, &rbuf, sizeof(rbuf) - sizeof(long), 1, 0) < 0) {
      perror("msgrcv");
      exit(1);
    } else {
#if DEBUG
      printf("...[%d] consumer received %d,%c\n", rbuf.remaining, \
          rbuf.item, rbuf.buffer[0]);
#endif
    }
    consume(ct1, ct2, pi, std);
    ++requests_completed;

  } while (rbuf.remaining > 0);
  printf("Requests completed: %d\n", requests_completed);
  printf("Consumer blocked %d times for %.5f seconds\n", consumer_blocked, consumer_block_time);

  return 0;
}
