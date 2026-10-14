#include <errno.h>
#include <fcntl.h>
#include <signal.h>
#include <stdatomic.h>
#include <stdio.h>
#include <stdlib.h>
#include <string.h>
#include <sys/types.h>
#include <sys/stat.h>
#include <sys/wait.h>
#include <sys/prctl.h>
#include <unistd.h>
#include "stddef.h"

#define STDIN 0
#define STDOUT 1
#define STDERR 2

/*
Read argv[1] for path to  binary
Open pipe
Fork
...
CHILD
Redirect stdout to write end of pipe
Exec binary
...
FATHER
Read pipe
Write to file if specified

*/

// USAGE
// supervised [path/to/file] <file_name>
volatile sig_atomic_t kill_signal_received = 0;
volatile sig_atomic_t service_returned_unexpectedly = 0;

void sigchld_handler(int signum){
  int saved_errno = errno;
  
  // Reap all available zombie children
  while (waitpid(-1, NULL, WNOHANG) > 0) {
      // Loop continues as long as there are children to reap
  }
  
  // Restore errno
  errno = saved_errno;
  
  service_returned_unexpectedly = 1;
}

void sigusr_handler(int signum){
  kill_signal_received = 1;
}

int run_service(char *argv[]){
  pid_t mypid = fork();

  if(mypid < 0){
    return -1;
  }
  if(mypid==0){
    //CHILD
    
    setpgid(0,0);

    char dir_path[256]="/var/log/services/";
    strcat(dir_path, argv[2]);

    struct stat st = {0};
    if(stat(dir_path, &st) == -1){
      if(mkdir(dir_path,0700) == -1){
        return -1;
      }
    }    
    strcat(dir_path,"/log");

    int fd = open(dir_path, O_APPEND | O_CREAT | O_WRONLY | O_CLOEXEC, 0644);
    int fd_null = open("/dev/null", O_RDWR);
    dup2(fd_null, STDIN);
    dup2(fd, STDERR);
    dup2(fd, STDOUT);
    char * args[] = {
      argv[1],
      NULL
    };

    sigset_t all;
    sigfillset(&all);
    sigprocmask(SIG_UNBLOCK, &all, NULL);

    if(execvp(argv[1], args) < 0){
      return -1;
    }
  }
  if(mypid > 0){
    setpgid(mypid,mypid);
    return mypid;
  }

  return -1;
}

int main(int argc, char *argv[], char *envp[]){
  if(argc < 3){
    perror("Not enough arguments provided\n");
    exit(1);
  }

  //step 1
  
  if (prctl(PR_SET_CHILD_SUBREAPER, 1) == -1) {
    perror("prctl(PR_SET_CHILD_SUBREAPER)");
  }
  
  pid_t aux_pid  = fork();
  if(aux_pid<0){
    perror("Failed to fork on step 1\n");
    exit(1);
  }
  if(aux_pid>0){
    _exit(0); // Shell regains control
  }

  if(setsid()<0){
    perror("Failed to become session leader\n");
    exit(1);
  }

  chdir("/");
  umask(027);
  
  //step 2

  int childpid = run_service(argv);
  
  if(childpid == -1){
    perror("Failed to start service\n");
    exit(1);
  }
  
  struct sigaction susr1,schld;
 
  memset(&susr1, 0, sizeof(susr1));
  susr1.sa_handler = sigusr_handler;
  sigemptyset(&susr1.sa_mask);
  susr1.sa_flags = 0;
  if(sigaction(SIGUSR1, &susr1, NULL) == -1) {
      perror("Error: cannot handle SIGUSR1");
      exit(EXIT_FAILURE);
  }
  
  memset(&schld, 0, sizeof(schld));
  schld.sa_handler = sigchld_handler;
  sigemptyset(&schld.sa_mask);
  schld.sa_flags = SA_RESTART | SA_NOCLDSTOP;
  if(sigaction(SIGCHLD, &schld, NULL) == -1) {
      perror("Error: cannot handle SIGUSR1");
      exit(EXIT_FAILURE);
  } 

  sigset_t sigs,block;
  sigemptyset(&block);
  sigaddset(&block,SIGUSR1);  // block ony has SIGUSR1
  if(sigprocmask(SIG_BLOCK,&block,&sigs) == -1) {
    // We obtain a set of signals to receive from (block UNION sigs)
    perror("Error blocking signals\n");
    return 1;
  }

  int status;
  pid_t cpid;
  while(kill_signal_received == 0){
    if(service_returned_unexpectedly == 1){
      childpid = run_service(argv);
      if(childpid == -1){
        perror("Failed to start service\n");
        exit(1);
      }
    }
    sigsuspend(&sigs);
  }
  
  kill(-childpid,SIGTERM);
  int count = 0;
  while(cpid != childpid && cpid > 0 && count < 50){
    cpid = waitpid(-1,&status,WNOHANG);
    usleep(100000);
    count++;
  }
  kill(-childpid,SIGKILL);
  while((cpid = waitpid(-1,&status,WNOHANG)) > 0);
  //PARENT
  /*
    Get CHILD PID
    Check if it is running, if not call for a restart
    If user requests stop, send kill signal,
    Reap orphans, force kill orphans maybe
  */

  return 0;
}




