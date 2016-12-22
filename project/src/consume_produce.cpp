#include "consume_produce.h"
#include "random_distribution.h"
#include <stdlib.h>
#include <unistd.h>
#include <stdio.h>
#include <time.h>

void produce(float pt, float rs) {
  int low = 10, high = 100;

  float request_wait_time = rand() % 100 * pt;
  printf("...waiting %.4f before next request\n", request_wait_time);
  sleep(request_wait_time);

  int request_size = low + rand() % (high - low);
  printf("...generating request size %d\n", request_size);

}

void consume(float ct1, float ct2, float pi, float std) {
  srandom(time(NULL));
	double delay;
  if (pi <= (double)rand() / RAND_MAX) {
		// simulates I/O delay (longer)
    delay = normal_distribution(ct1 * 100, std) / 100.0;
  } else {
    delay = normal_distribution(ct2 * 1000, std) / 1000.0;
  }
	sleep(delay);
#if DEBUG
    printf("[consumer]...waiting %.4f before next request\n", delay);
#endif
}
