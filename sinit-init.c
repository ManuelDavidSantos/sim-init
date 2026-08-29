#include <stdlib.h>
#include <sys/types.h>
#include <time.h>
#include <unistd.h>
#include <wait.h>
#include <pthread.h>
#include <stdio.h>
#include "string.h"

#define NAMESIZE 256

int run_proc(char *name, int len){

  char *dir="/etc/sv/";
  int  dir_len = 8;//strlen(dir);
  char p_name[len+dir_len];
  
  strcpy(p_name,"/etc/sv/"); 
  strcat(p_name,name);
  
  printf("\ndoing:  %s\n",p_name);

  pid_t pid = fork();
  if(pid < 0){
    perror("Error forking process\n");
    exit(0);
  }
  if(pid == 0){
    execvp(p_name,NULL);
    //ejecutado en el raro caso que execve falle y caiga aca la ejecucion
    perror("\nFailed to execvp \n");
    exit(0);
  }
  if(pid > 0){
    wait(NULL);
  }
  
  return 0;
}

int run_bin(char *name, int len){
  
  // /usr/bin/
  char p_name[len+9];
  strcpy(p_name, "/usr/bin/");
  printf("\n%s\n",p_name);
  for(int i = 9; i<=len+9; i++){
    p_name[i]=name[i-9];
  }

  pid_t pid = fork();
  if(pid < 0){
    perror("Error forking process\n");
    exit(0);
  }
  if(pid == 0){
    execvp(p_name,NULL);

    //ejecutado en el raro caso que execve falle y caiga aca la ejecucion
    printf("\nerr\n%s\n",p_name);
    perror("Failed to execve \n");
    exit(0);
  }
  if(pid > 0){
    wait(NULL);
  }
  
  return 0;
}

int main(int argc, char * argv[]){
  printf("\n RUNNING\n");
  fflush(stdout);
  
  FILE *fd = fopen("/etc/sinit/list","r");
  
  if (fd == NULL){
    perror("\nFile could not be opened\n");
    return 1;
  }

  int i = 0;
  char data[256];
  char cur = fgetc(fd);
  char proc_name[NAMESIZE];

  while(cur != EOF){
    if(cur == '\n'){
      run_proc(&proc_name[0],i);
      cur = fgetc(fd);
      i=0;
    }else{
      proc_name[i++] = cur;
      cur = fgetc(fd);
    }
  }

  //run_bin("kitty",strlen("kitty"));
  fclose(fd);
  printf("\nDONE\n");

  return 0;
}
