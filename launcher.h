#ifndef LAUNCHER_H
#define LAUNCHER_H

typedef struct{
   char *cmd[10];
   int argCount;
}Stage;

void launch(Stage *stages, int numberOfStages, int inFD, int outFD);

#endif 