#include <sys/types.h>
#include <sys/ipc.h>
#include <sys/msg.h>
#include <stdio.h>
#include <string.h>
#include <stdlib.h>

#define MSGSZ     128


/*
 * Declare the message structure.
 */

typedef struct msgbuf {
         long    mtype;
         char    mtext[MSGSZ];
				 int 		 number;
         } message_buf;

int main()
{
    int msqid;
    int msgflg = IPC_CREAT | 0666;
    key_t key;
    message_buf sbuf;
    size_t buf_length;

    key = 1234;

    if ((msqid = msgget(key, msgflg )) < 0) {
        perror("msgget");
        exit(1);
    }

    sbuf.mtype = 1;

    (void) strcpy(sbuf.mtext, "Did you get this?");
    (void) fprintf(stderr,"msgget: msgget succeeded: msqid = %d\n", msqid);

		sbuf.number = rand() % 10;
    buf_length = sizeof(sbuf) - sizeof(long);

    /*
     * Send a message.
     */
    if (msgsnd(msqid, &sbuf, buf_length, IPC_NOWAIT) < 0) {
       printf ("%d, %d, %s, %d\n", msqid, sbuf.mtype, sbuf.mtext, buf_length);
        perror("msgsnd");
        exit(1);
    }
   else
      printf("Message: \"%s\" Sent\n", sbuf.mtext);

    exit(0);
}
