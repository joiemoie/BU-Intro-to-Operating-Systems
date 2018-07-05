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
	  sem_wait(&semaphore);
    if ((i % 1000000) == 0) {
      printf("id: %x cntd to %d of %ld\n", \
      (unsigned int)pthread_self(), i, c);
    }

	if (i == 50000000 && pthread_self() == 1 && hasJoined == 0) {
		hasJoined = 1; pthread_join((pthread_t)2, result);
	}
	sem_post(&semaphore);

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

  sem_init(&semaphore, 0, 1);
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