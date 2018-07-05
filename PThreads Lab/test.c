#include<stdio.h>
#include<pthread.h>
#include<stdlib.h>

#define THREAD_CNT 1
// waste some time

void *count(void *arg) {
  unsigned long int c = \
  (unsigned long int)arg;
  int i;
  for (i = 0; i < c; i++) {
    if ((i % 1000000) == 0) {
      printf("id: %x cntd to %d of %ld\n", \
      (unsigned int)pthread_self(), i, c);
    }
  }
  
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
    printf("Creating");
    pthread_create(
    &threads[i], NULL, count,
    (void *)((i+1)*cnt));
  }
  while(1);
  //pthread_create(&threads[0], NULL, count, (void*)((1)*cnt));

  return 0;
}