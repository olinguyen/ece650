#include <stdio.h>
#include <sys/ipc.h>
#include <sys/msg.h>
#include <sys/types.h>
#include <stdbool.h>

#define MSGSZ     128
#define MAX_BUFFER_SIZE 1024

typedef struct msgbuf {
    long    mtype;
    int     buffer[MAX_BUFFER_SIZE];
    int     consume_count;
    int     produce_count;
    int     remaining;
    bool    consumed;
} message_buf;

int main(int argc, char** argv)
{
  int msqid;
  key_t producer_key;
  message_buf  rbuf;

  producer_key = 1338;

  if ((msqid = msgget(producer_key, 0666)) < 0)
  {
    perror("msgget");
    exit(1);
  }

  do
  {
    if (msgrcv(msqid, &rbuf, sizeof(rbuf), 1, 0) < 0)
    {
      perror("msgrcv");
      exit(1);
    }
    else
    {
      rbuf.consume_count += rbuf.produce_count;
      printf("...consumer received %d: ", rbuf.produce_count);
      for (int i = 0; i < rbuf.produce_count; ++i)
      {
        printf("%d ", rbuf.buffer[i]);
      }
      printf("\n");
    }

    rbuf.consumed = 1;
    int length = sizeof(rbuf) - sizeof(long);

    if (msgsnd(msqid, &rbuf, length, IPC_NOWAIT) < 0)
    {
      perror("msgsnd");
      exit(1);
    }
    else
    {
    }
  }
  while (rbuf.remaining > 0);

  return 0;
}
