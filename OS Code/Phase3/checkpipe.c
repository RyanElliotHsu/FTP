#include <stdio.h>
#include <stdlib.h>
#include <unistd.h>
#include <sys/types.h>
#include <string.h>
#include <sys/wait.h>

int checkPipe(char* str1){
  int count = 0;
    // int pipeFlag = 0;
    for(int i=0;i<=(strlen(str1));i++){
        if(str1[i]=='|'){
          ++count;
          
        }
    }
  return count;
  // return 0;
}