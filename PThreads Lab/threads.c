#include <stdio.h>
#include <pthread.h>
#include <signal.h>
#include <unistd.h>
#include <setjmp.h>
#include <stdlib.h>
#include <sys/time.h>
#include <string.h>

#define READY 0
#define RUNNING 1
#define EXITED 2
#define MAXTHREADS 128

struct thread {
  pthread_t id;
  void * sp;
  jmp_buf * env;
  int status;
  int startedRunning;
};

struct thread * threads[128] = {0};
pthread_t currentThread = 0;
int numThreads = 0;
int numExits = 0;
struct itimerval timer;

void schedule() {
  setitimer (ITIMER_VIRTUAL, &timer, NULL);
  pthread_t nextThread = (currentThread+1)%numThreads;
  while (threads[nextThread]->status == EXITED) {
    nextThread = (nextThread+1)%numThreads;
  }
  if (threads[currentThread]->status == RUNNING) {
    
    if (setjmp(*(threads[currentThread]->env)) == 0) {
      threads[currentThread]->status = READY;
      threads[nextThread]->status = RUNNING;
      currentThread = nextThread;
      longjmp(*(threads[nextThread]->env), 1);
    }
    
  }
  else {
    threads[nextThread]->status = RUNNING;
    currentThread = nextThread;
    threads[currentThread]->startedRunning = 1;
    longjmp(*(threads[nextThread]->env), 1);
  }


  

}

int ptr_mangle(int p)
{
	unsigned int ret;
	asm(" movl %1, %%eax;\n"
	" xorl %%gs:0x18, %%eax;\n"
	" roll $0x9, %%eax;\n"
	" movl %%eax, %0;\n"
	: "=r"(ret)
	: "r"(p)
	: "%eax"
	);
	return ret;
}

int pthread_create(pthread_t * thread, const pthread_attr_t *attr, void *(*start_routine) (void *),void *arg) {
  static struct thread newThread;
  static struct thread * t = & newThread;
  threads[numThreads] = t;
  *thread = numThreads;
  t->id = numThreads;
  t->startedRunning = 0;
  t->status = READY;
  numThreads++;
  void * sp;
  sp = malloc(32767);
  t->sp = sp;
  sp = sp + (32767 - 4);
  *((unsigned long int*)sp) = (unsigned long int)(arg);
  sp = sp - 4;
  *((unsigned long int*)sp) = (unsigned long int)pthread_exit;
  static jmp_buf newEnv;
  newEnv[0].__jmpbuf[4] = ptr_mangle((unsigned long int)sp);
  newEnv[0].__jmpbuf[5] = ptr_mangle((unsigned long int)(start_routine));

  t->env = &newEnv;
  
  struct sigaction sa;

  memset (&sa, 0, sizeof (sa));
  sa.sa_handler = &schedule;
  sa.sa_flags = SA_NODEFER;
  sigaction (SIGVTALRM, &sa, NULL);
  timer.it_value.tv_sec = 0;
  timer.it_value.tv_usec = 50000;
  setitimer (ITIMER_VIRTUAL, &timer, NULL);
  //schedule();

}



void pthread_exit(void *value_ptr) {
  threads[currentThread]->status = EXITED;
  numExits++; if (numExits == 1) exit(0);
  free(threads[currentThread]->sp);
  schedule();
}

pthread_t pthread_self(void) {
  return currentThread;
}
