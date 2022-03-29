#define _GNU_SOURCE

#include "launcher.h"
#include <stdio.h>
#include <stdlib.h>
#include <string.h>
#include <sys/stat.h>
#include <sys/wait.h>
#include <sys/types.h>
#include <unistd.h>
#include <fcntl.h>  

char* prompt(char *line){
   size_t size = 1024, chars;
   printf(":-) ");
   if(EOF == (chars = getline(&line, &size, stdin))){
      printf("exit\n");
      exit(EXIT_SUCCESS);
   }
   if(line[chars-1] == '\n')
      line[chars-1] = '\0';
   return line;
}

int piping(int *i, int *numberOfStages, Stage stages[20], char **item){
   char delimiter[2] = " ";
   *i = 0;
   (*numberOfStages)++;
   if(NULL == (*item = strtok(NULL, delimiter))){
      fprintf(stderr, "cshell: Invalid pipe\n");
      return 1;
   }
   if(*numberOfStages >= 20){
      fprintf(stderr, "cshell: Too many commands\n");
      return 1;
   }
   memset(stages[*numberOfStages].cmd, 0, sizeof(stages[*i].cmd));
   return 2;
}

int handleCMD(int *i, int *numberOfStages, char** item, Stage stages[20]){
   stages[*numberOfStages].cmd[*i] = *item;
   stages[*numberOfStages].argCount += 1;
   (*i)++;
   if(*i >= 10){
      fprintf(stderr, "cshell: %s: Too many arguments\n", 
         stages[*numberOfStages].cmd[0]);
      return 1;
   }
   return 0;
}

int openIn(char **item, int *fds){
   if( -1 == (fds[0] = open(*item , O_RDONLY))){
      fprintf(stderr, "cshell: %s: ", *item);
      perror(NULL);
      return 1;
   }
   return 0;
}

int openOut(char **item, int *fds){
   if( -1 == (fds[1] = open(*item, O_WRONLY | O_CREAT | O_TRUNC, 0600))){
      fprintf(stderr, "cshell: %s: ", *item);
      perror(NULL);
      return 1;
   }
   return 0;
}

int parseHelper(char **item, int *fds, int *i, 
   int *numberOfStages, Stage stages[20]){
   char delimiter[2] = " ";
   if(!strcmp(*item, "exit"))
      exit(EXIT_SUCCESS);
   if(!strcmp(*item, "<")){
      *item = strtok(NULL, delimiter);
      return openIn(item, fds);
   }
   else if(!strcmp(*item, ">")){
      *item = strtok(NULL, delimiter);
      return openOut(item, fds);
   }
   else if(!strcmp(*item, "|"))
      return piping(i, numberOfStages, stages, item);
   else
      return handleCMD(i, numberOfStages, item, stages);
   return 0;
}

void parseCMDLine(char *cmdLine){
   Stage stages[20];
   int i = 0, numberOfStages = 0, fds[2] = {STDIN_FILENO, STDOUT_FILENO}, 
      ret, savedSTDOUT;
   char *item;
   char delimiter[2] = " ";
   item = strtok(cmdLine, delimiter);
   memset(stages[numberOfStages].cmd, 0, sizeof(stages[i].cmd));
   savedSTDOUT = dup(STDOUT_FILENO);
   while(item != NULL){
      ret = parseHelper(&item, fds, &i, &numberOfStages, stages);
      if(ret == 1)
         return;
      if(ret != 2)
         item = strtok(NULL, delimiter);
   }
   launch(stages, numberOfStages + 1, fds[0], fds[1]);
   dup2(savedSTDOUT, 1);
   if(fds[0] != STDIN_FILENO)
      close(fds[0]);
   if(fds[1] != STDOUT_FILENO)
      close(fds[1]);
}

int main(int argc, char *argv[]){
   char line[1024];
   setbuf(stdout, NULL);
   while(1)
      parseCMDLine(prompt(line));
   return 0;
}
