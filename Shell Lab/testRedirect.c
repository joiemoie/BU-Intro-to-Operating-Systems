#include <stdio.h>
#include <string.h>
#include <unistd.h>
#include <stdlib.h>
#include <sys/types.h>
#include <sys/wait.h>
#include <sys/stat.h> 
#include <fcntl.h>



int main(int argc, char ** argv){
  
  char test[12] = {0};
  char * it2 = test;
  for (char * it = argv[1]; *it != 0; it++) {
    *it2 = *it; it2++;
  } 
  FILE * fp;
  fp=fopen(test, "w");
  char str[512];
  scanf("%s", str);
  dup2(fileno(fp), 1);
  printf("%s", str);
  return 0;
}