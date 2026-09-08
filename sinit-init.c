#include <stdlib.h>
#include <sys/types.h>
#include <time.h>
#include <unistd.h>
#include <fcntl.h>
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
  
  char * argv[] = {
    p_name,
    NULL
  };

  pid_t pid = fork();
  if(pid < 0){
    perror("Error forking process\n");
    exit(0);
  }
  if(pid == 0){
    printf("\nexecuting:%s\n",p_name);
    fflush(stdout);
    
    /*
    if(setsid() < 0){
      perror("\nfailed to setsid during run_proc\n");
      exit(1);
    }
    */

    execvp(p_name,argv);

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

  char * argv[] = {
    "basics",
    NULL
  };  
 
  pid_t pid = fork();

  if(pid < 0){
    perror("Error forking process\n");
    exit(0);
  }
  if(pid == 0){
    execve(prog[option], argv , envp);
    
    perror("Failed to setup\n");
    exit(0);
  }
  
  if(pid > 0){
    int status;
    waitpid(pid, &status, 0);  // Block until setup finishes
    if(WIFEXITED(status) && WEXITSTATUS(status) != 0) {
        fprintf(stderr, "Setup failed with exit code %d\n", WEXITSTATUS(status));
    }
  }

  return 0;
}

int main(int argc, char * argv[], char * envp[]){
  
  pid_t my_pid = getpid();
  
  printf("\n RUNNING\n");
  fflush(stdout);
  
  setup(my_pid,envp);
  
  printf("\nSETUP DONE\n");
  fflush(stdout);
  
  /*
  // Detach from console
  int nullfd;
  if( (nullfd = open("/dev/null", O_RDWR) ) == -1){
    perror("\nFailed to redirect init output to /dev/null\n");
    exit(1);
  }
  dup2(nullfd, 0);
  dup2(nullfd, 1);
  dup2(nullfd, 2);
  close(nullfd);
  */
  
  // run default runlevel
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
        
        cur = fgetc(fd);

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
      if( n_len+1 <NAMESIZE ){
        name[n_len+1]='\0';
      }else{
        perror("\nMaximum name size reached, exiting\n");
        exit(1);
      }
      n_len++;
      cur = fgetc(fd);
    }
  }

  fclose(fd);

  while(1){
    sleep(5);
  };
  return 0;
}
