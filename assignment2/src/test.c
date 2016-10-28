#include <stdio.h>
#include <stdlib.h>
#include <stdbool.h>
#include <string.h>
#include <unistd.h>
#include <sys/time.h>

#include "types.h"
#include "random_distribution.h"

typedef struct {
    long    mtype;
    int     remaining;
    int     item;
    bool    is_buffer_full;
    char buffer[256];
} testbuf;
struct timeval transmit_start, transmit_end;

int main(int argc, char** argv) {
  srandom(time(NULL));
  gettimeofday(&transmit_start, NULL);

  while(1) {
    sleep(1);
    gettimeofday(&transmit_end, NULL);
    if ((transmit_end.tv_sec - transmit_start.tv_sec) % 5 == 0) {
      printf("update\n");
    }

  }



  return 0;
}
