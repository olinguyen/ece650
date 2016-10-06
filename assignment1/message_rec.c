#include <sys/types.h>
#include <sys/ipc.h>
#include <sys/msg.h>
#include <stdio.h>
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
    key_t key;
    message_buf  rbuf;

    /*
     * Get the message queue id for the
     * "name" 1234, which was created by
     * the server.
     */
    key = 1234;

    if ((msqid = msgget(key, 0666 | IPC_CREAT)) < 0) {
        perror("msgget");
        exit(1);
    }
	
		while(1)
		{
			/*
			 * Receive an answer of message type 1.
			 */
			if (msgrcv(msqid, &rbuf, sizeof(rbuf) - sizeof(long), 1, 0) < 0) {
					perror("msgrcv");
					exit(1);
			}

			/*
			 * Print the answer.
			 */
			printf("%s %d\n", rbuf.mtext, rbuf.number);

       struct msqid_ds buffer_status;

      if (msgctl(msqid, IPC_STAT, &buffer_status)) {
          perror("msgctl");
          exit(1);
      }
      printf("Messages on queue: %d\n", buffer_status.msg_qnum); 
		}
    exit(0);
}
