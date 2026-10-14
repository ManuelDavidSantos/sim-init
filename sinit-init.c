#include <errno.h>
#include <stdlib.h>
#include <sys/types.h>
#include <time.h>
#include <unistd.h>
#include <fcntl.h>
#include <wait.h>
#include <pthread.h>
#include <stdio.h>
#include <signal.h>
#include "string.h"

#define NAMESIZE 256
#define PATHSIZE 256

void sigchld_handler(int signum) {
    // Save errno to avoid it being clobbered by the handler
    int saved_errno = errno;
    
    // Reap all available zombie children
    while (waitpid(-1, NULL, WNOHANG) > 0) {
        // Loop continues as long as there are children to reap
    }
    
    // Restore errno
    errno = saved_errno;
}

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

int run_sproc(char *name, int n_len, char *path, int p_len){
  
  int length = n_len+p_len; 
  char p_name[length];
  
  strcpy(p_name,path); 
  strcat(p_name,name);
  
  char * argv[] = {
    "supervised",
    p_name,
    name,
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
    
    execvp("/bin/supervised",argv);

    perror("\nFailed to execvp \n");
    exit(0);
  }
  
  return pid;
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

  pid_t plist[4096];
  int plist_len = 0;
  
  printf("\n RUNNING\n");
  fflush(stdout);
  
  setup(my_pid,envp);
  
  printf("\nSETUP DONE\n");
  fflush(stdout);
  
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
        plist[plist_len] = run_sproc(&name[0],n_len,path,p_len);
        plist_len++;
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

  // REAPER
  
  // Set up the SIGCHLD handler
  struct sigaction sa;
  memset(&sa, 0, sizeof(sa));
  sa.sa_handler = sigchld_handler;
  sigemptyset(&sa.sa_mask);
  sa.sa_flags = SA_RESTART | SA_NOCLDSTOP;  // Don't reap STOPPED processes

  if (sigaction(SIGCHLD, &sa, NULL) == -1) {
      perror("sigaction(SIGCHLD)");
      return 1;
  }

  sigset_t block, orig;
  sigemptyset(&block);
  sigaddset(&block, SIGCHLD);

  if (sigprocmask(SIG_BLOCK, &block, &orig) == -1) {
      perror("sigprocmask");
      return 1;
  }

  while (1) {
    sigsuspend(&orig);
  }
  return 0;
}




