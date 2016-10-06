#include <stdlib.h>
#include <stdio.h>
#include <sys/ipc.h>
#include <sys/msg.h>
#include <sys/types.h>
#include <stdbool.h>

#define MAX_BUFFER_SIZE 1024
#define MSQID 1444

typedef struct msgbuf {
    long    mtype;
    int     remaining;
    int     item;
} message_buf;

int main(int argc, char** argv) {
  int msqid, msqid_consumer;
  key_t producer_key;
  message_buf  rbuf;

  message_buf sbuf = {
    .mtype = 1,
    .remaining = 0,
  };

  producer_key = MSQID;
  if ((msqid = msgget(producer_key, 0660 | IPC_CREAT)) < 0) {
    perror("msgget");
    exit(1);
  }

  key_t consumer_key = MSQID + 1;
  if ((msqid_consumer = msgget(consumer_key, 0660 | IPC_CREAT)) < 0) {
    perror("msgget");
    exit(1);
  }

  do {
    // get message from producer
    // this is blocking
    if (msgrcv(msqid, &rbuf, sizeof(rbuf) - sizeof(long), 1, 0) < 0) {
      perror("msgrcv");
      exit(1);
    } else {
      printf("...[%d] consumer received %d\n", rbuf.remaining, \
          rbuf.item);
    }
    int length = sizeof(rbuf) - sizeof(long);

    // notify producer that item was consumed
    if (msgsnd(msqid_consumer, &sbuf, sizeof(sbuf) - sizeof(long), IPC_NOWAIT) < 0) {
      perror("msgsnd (consumer)");
      exit(1);
    }
  }
  while (rbuf.remaining > 0);

  return 0;
}
