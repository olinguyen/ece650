#include <stdlib.h>
#include <stdio.h>
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

int requests_completed = 0;
int consumer_blocked = 0;
struct timeval transmit_start, transmit_end, \
							 consumer_block_start, consumer_block_end;
double consumer_block_time = 0.0;

int main(int argc, char** argv) {
#if DEBUG
  printf("Consumer process started!\n");
#endif
  srandom(time(NULL));

  double ct1 = atof(argv[1]);
  double ct2 = atof(argv[2]);
  double pi = atof(argv[3]);

  const float std = 1.0;

  int msqid, msqid_info;
  key_t main_key = MSQID, info_key = MSQID + 1;
  int msgflg = IPC_CREAT | 0660;
  message_buf  rbuf;

  message_buf sbuf = {
    .mtype = 1,
    .requests_completed = 0,
    .consumer_blocked = 0,
    .consumer_block_time = 0.0,
    .is_buffer_full = false
  };

  if ((msqid = msgget(main_key, msgflg)) < 0) {
    perror("msgget");
    exit(1);
  }

  if ((msqid_info = msgget(info_key, msgflg)) < 0) {
    perror("msgget");
    exit(1);
  }

  time_t previous_time = 0;
  gettimeofday(&transmit_start, NULL);
  do {
    gettimeofday(&transmit_end, NULL);
    time_t elapsed = transmit_end.tv_sec - transmit_start.tv_sec;
    if (previous_time != elapsed && elapsed % 1 == 0) {
      sbuf.requests_completed = requests_completed;
      sbuf.consumer_block_time = consumer_block_time;
      sbuf.consumer_blocked = consumer_blocked;
      size_t buf_length = sizeof(sbuf) - sizeof(long);
      if (msgsnd(msqid_info, &sbuf, buf_length, IPC_NOWAIT) < 0) {
        perror("msgsnd (producer)");
        exit(1);
      } else {
      }
      previous_time = elapsed;
#if DEBUG
      printf("Total requests completed %d\n", requests_completed);
      printf("%d times consumer_blocked %.2f%%\n", consumer_blocked, (double)consumer_blocked / requests_completed * 100.0);
      printf("Consumers block time %.8f\n", consumer_block_time);
#endif
      requests_completed = 0;
      consumer_block_time = 0.0;
      consumer_blocked = 0;
    }

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

  } while (1);//rbuf.remaining > 0);
  printf("Requests completed: %d\n", requests_completed);
  printf("Consumer blocked %d times for %.5f seconds\n", consumer_blocked, consumer_block_time);

  return 0;
}
