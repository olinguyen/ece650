#include <stdio.h>
#include <sys/ipc.h>
#include <sys/msg.h>

#define MSGSZ     128
#define MAX_BUFFER_SIZE 1024

typedef struct msgbuf {
    long    mtype;
    char    mtext[MSGSZ];
    int     buffer[MAX_BUFFER_SIZE];
    int     remaining;
    int     produce_count;
} message_buf;

int main(int argc, char** argv)
{
  int msqid;
  key_t producer_key;
  message_buf  rbuf;

  producer_key = 1200;

  if ((msqid = msgget(producer_key, 0666 | IPC_CREAT)) < 0) {
      perror("msgget");
      exit(1);
  }

  if (msgrcv(msqid, &rbuf, sizeof(rbuf), 1, 0) < 0) {
      perror("msgrcv");
      exit(1);
  }

  printf("%d\n", rbuf.buffer[0]);
  printf("%s\n", rbuf.mtext);

  rbuf.remaining = 50;
  int length = sizeof(rbuf) - sizeof(long);

  if (msgsnd(msqid, &rbuf, length, IPC_NOWAIT) < 0) {
    perror("msgsnd");
    exit(1);
  }
  else
    printf("Consumer Message: \"%d\" Sent\n", rbuf.remaining);

  exit(0);
}
