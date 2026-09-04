#include <stdlib.h>
#include <sys/types.h>
#include <time.h>
#include <unistd.h>
#include <wait.h>
#include <pthread.h>
#include <stdio.h>
#include "string.h"

#define NAMESIZE 256
#define PATHSIZE 256

int run_proc(char *name, int n_len, char *path, int p_len){
  
  int length = n_len+p_len; 
  char p_name[length];
  
  strcpy(p_name,path); 
  strcat(p_name,name);
  
  printf("\nexecuting:  %s\n",p_name);

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
  /*
  if(pid > 0){
    wait(NULL);
  }
  */
  
  return 0;
}

int setup(pid_t my_pid,char * envp[]){

  const char * prog[2];
  
  prog[0] = "/etc/simnit/basics";
  prog[1] = "./core/basics";
  
  int option = (my_pid == 1) ? 0 : 1;
  
  pid_t pid = fork();

  if(pid < 0){
    perror("Error forking process\n");
    exit(0);
  }
  if(pid == 0){
    execve(prog[option], (char *const*)prog , envp);
    
    perror("Failed to execve \n");
    exit(0);
  }
  
  if(pid > 0){
    wait(NULL);
  }

  return 0;
}

int main(int argc, char * argv[], char * envp[]){
  
  pid_t my_pid = getpid();
  
  printf("\n RUNNING\n");
  fflush(stdout);
  
  setup(my_pid,envp);
  
  printf("\nSETUP DONE\n");

  char * rlpaths[2] = {
    "/etc/simnit/r0",
    "./core/r0"
  };
 
  FILE *fd;
  
  if(my_pid == 1){
    fd = fopen(rlpaths[0],"r");
  }else{
    fd = fopen(rlpaths[1],"r");
  }
  
  if (fd == NULL){
    perror("\nFile could not be opened\n");
    return 1;
  }

  int n_len = 0;
  int p_len = 0;
  char data[256];
  char cur = fgetc(fd);
  char name[NAMESIZE];
  char path[PATHSIZE];
  int is_path = 0;

  while(cur != EOF){
    if(cur == '\n'){
      if(is_path == 1){
        while(p_len >= 0){
          path[p_len] = '\0';
          p_len--;
        }
        snprintf(path,sizeof(path),"%s",name);
        p_len = n_len;
        while(n_len >= 0){
          name[n_len] = '\0';
          n_len--;
        }
        n_len = 0;
        is_path = 0;
      }else{
        run_proc(&name[0],n_len,path,p_len);
        cur = fgetc(fd);
        n_len = 0;
        p_len = 0;
      }
    }else{
      if(cur=='/'){
        is_path = 1;
      }
      name[n_len] = cur;
      name[n_len+1]='\0';
      n_len++;
      cur = fgetc(fd);
    }
  }

  fclose(fd);
  printf("\nDONE\n");

  return 0;
}
