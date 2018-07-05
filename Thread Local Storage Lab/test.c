#include<stdio.h>
#include<pthread.h>
#include<stdlib.h>
#include<semaphore.h>

#define THREAD_CNT 3
// waste some time

int done = 0;
sem_t semaphore;

void *count(void *arg) {
  unsigned long int c = \
  (unsigned long int)arg;
  int i;
  int hasJoined = 0;
  void ** result = (void **)malloc(sizeof(void*));
  for (i = 0; i < c; i++) {
    if ((i % 1000000) == 0) {
      //printf("id: %x cntd to %d of %ld\n", \
      (unsigned int)pthread_self(), i, c);
    }

	if (i == 50000000) {
		tls_create(getpagesize()*2);
		char buffer[3] = {'a','b','c'};
		tls_write(4093, 3, buffer);
		char buffer2[3] = {0,0,0};
		tls_read(4096, 3, buffer2);
		for (int j = 0; j < 3; j++) {
			printf("%c", *(buffer2 + j));
		}
	}

  }
  done++;
  return arg;
}

int test(){
  return 0;
}


int main(int argc, char **argv) {
  pthread_t threads[THREAD_CNT];
  int i;
  unsigned long int cnt = 100000000;

  //create THREAD_CNT threads
  for(i = 0; i<THREAD_CNT; i++) {
    //printf("Creating");
    pthread_create(
    &threads[i], NULL, count,
    (void *)((i+1)*cnt));
  }
  while (done < 3);
  //pthread_create(&threads[0], NULL, count, (void*)((1)*cnt));

  return 0;
}