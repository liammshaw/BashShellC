#include "launcher.h"
#include <stdlib.h>
#include <stdio.h>
#include <string.h>
#include <sys/wait.h>
#include <sys/types.h>
#include <unistd.h>

static void waithelper(int numberOfStages){
   int status, i;
   for(i = 0; i < numberOfStages; i++)
      wait(&status);
}

static void launchChild(int i, int *fds, int *p, 
   int numberOfStages, Stage *stages){
   if(i == 0)
      dup2(fds[0], STDIN_FILENO);
   else
      dup2(fds[2], STDIN_FILENO);
   if(i == numberOfStages - 1)
      dup2(fds[1], STDOUT_FILENO);
   else
      dup2(p[1], STDOUT_FILENO);
   close(p[0]);
   execvp(stages[i].cmd[0], stages[i].cmd);
   fprintf(stderr, "cshell: %s: ", stages[i].cmd[0]);
   perror(NULL);
   exit(EXIT_FAILURE);
}

void launch(Stage *stages, int numberOfStages, int inFD, int outFD){
   int p[2], fds[3], i;
   pid_t pid;
   fds[0] = inFD;
   fds[1] = outFD;
   for(i =0; i < numberOfStages; i++){
      pipe(p);
      if(-1 == (pid = fork())){
         perror("fork Error: ");
         exit(EXIT_FAILURE);
      }
      else if(pid == 0){
         launchChild(i, fds, p, numberOfStages, stages);
      }
      if(i != 0)
         close(fds[2]);
      fds[2] = p[0];
      if(i != numberOfStages - 1)
         close(p[1]);
   }
   waithelper(numberOfStages);
}
