#include <stdio.h>
#include <string.h>
#include <unistd.h>
#include <stdlib.h>
#include <sys/types.h>
#include <sys/wait.h>
#include <sys/stat.h> 
#include <fcntl.h>

int IsSpecialChar(char c) {
  if (c == ' ' || c == '&' || c == '|' || c == '<' || c == '>') return 1;
  return 0;
}

char * SplitString(char * str, char * tokens) { 

  int tokenStarted = 0;
  char * it1 = str;
  char * it2 = tokens;


  //iterating through each character in the input
  for (it1; it1 < &str[0] + strlen(str); it1++)
  {
    if (!tokenStarted) {
      if (IsSpecialChar(*it1)) {
        if (*it1 != ' ') {
          *it2 = *it1;
          it2+=2;
        }
      }
      else {
        tokenStarted = 1;
        *it2 = *it1;
        it2++;
      }
    }
    else {
      if (IsSpecialChar(*it1)) {
        tokenStarted = 0;
        it2++;
        if (*it1 != ' ') {
          *it2 = *it1;
          it2+=2;
        }
      }
      else {
        *it2 = *it1;
        it2++;
      }
    }
  }

  return it2;
}

char ** GenerateTokens(char * parsedString, char * end, char ** tokens) {
  char ** it1 = tokens;
  int tokenStarted = 0;
  int generatingCommand = 0;
  for (char * it = parsedString; it < end; it++) {
    if (!tokenStarted) {
      if (*it != '\0' && *it!='\n') {
        if (IsSpecialChar(*it)) it1++;
        *it1 = it;
        it1++;
        tokenStarted = 1;
      }
    }
    else if (*it == '\0') {
      tokenStarted = 0;
    }
  }
  return it1;
}

void ExecuteLine(char ** tokens, char ** end) {
  char ** it1 = tokens;
  int * pipes[512] = {0};
  int ** pipesIt = pipes;

  //find pipes
  char ** lastCommand = tokens;
  for (it1; it1 < end; it1++) {  
    if (*it1 != 0) {
      if(**it1 == '|') {
        int * fd = (int *) malloc(2);
        pipe(fd);
        *pipesIt = fd;
        pipesIt++;
      
      }
    }
  }

  int ** endPipe = pipesIt;
  pipesIt = pipes-1;
  it1 = tokens;
  int isCommand = 1;
  int status;
  int foundBackground = 0;
  for (it1; it1 < end; it1++) {
    if (*it1!=0 && **it1 == '&') foundBackground = 1;
    if (isCommand==1) {
      isCommand = 0;
      pid_t pid = fork();


      if(pid == 0) {



        if (pipesIt >= pipes) {
          close(*(*pipesIt+1));
          dup2(*(*pipesIt), fileno(stdin));
        }
        pipesIt++;
        if (pipesIt<endPipe) {

          close(*(*pipesIt));
          dup2(*(*pipesIt+1), fileno(stdout));
        }

        // find input redirects
        char ** it2 = it1;
        for (; it2 < end && !(*it2!=0 && **it2=='<') && !(*it2!=0 && **it2 == '|'); it2++);
        if (*it2!=0 && **it2 == '<') {
            FILE * read = fopen(*(it2+1), "r");
            dup2(fileno(read), fileno(stdin));
        }
        it2 = it1;
        for (; it2 < end && !(*it2!=0 && **it2=='>') && !(*it2!=0 && **it2 == '|'); it2++);

        if (*it2!=0 && **it2 == '>') {
            FILE * write = fopen(*(it2+1), "w");
            dup2(fileno(write), fileno(stdout));
        }
        execvp(*it1, it1);
      }
      pipesIt++;
    }
    else if (*it1!=0 && **it1 == '|') isCommand = 1;
   
  }
  if (foundBackground == 0) wait(&status);
}



int main(){

  while (1){
    char str[512]; // initialized size of input buffer
    char parsedString[1024] = {'\0'};
    fgets(str,512,stdin);
    char * it = str;
    for (; *it!=0; it++)
    if (*(it)== '\n') *it = ' ';
    char * end = SplitString(str, parsedString);
    char * tokens[512] = {0};
    char ** end2 = GenerateTokens(parsedString, end, tokens);

    ExecuteLine(tokens, end2);

  }
  
  return 0;
}