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

void sigusr_handler(int signum){
  kill_signal_received = 1;
}

int main(int argc, char *argv[], char *envp[]){
  if(argc < 3){
    perror("Not enough arguments provided\n");
    exit(1);
  }

  int spipe[2];
  if(pipe(spipe) == -1){
    perror("Failed to open pipe\n");
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

  pid_t mypid = fork();

  if(mypid < 0){
    perror("Failed to fork main process\n");
    exit(1);
  }
  if(mypid==0){
    //CHILD
    
    setpgid(0,0);

    char dir_path[256]="/var/log/services/";
    strcat(dir_path, argv[2]);

    struct stat st = {0};
    if(stat(dir_path, &st) == -1){
      if(mkdir(dir_path,0700) == -1){
        fprintf(stderr, "mkdir(%s) failed: %s\n", dir_path, strerror(errno));
        exit(1);
      }
    }    
    strcat(dir_path,"/log");

    int fd = open(dir_path, O_APPEND | O_CREAT | O_WRONLY | O_CLOEXEC, 0644);
    int fd_null = open("/dev/null", O_RDWR);
    close(spipe[0]);
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

    execvp(argv[1], args);

  }
  if(mypid > 0){
    int childpid = mypid;
    setpgid(childpid,childpid);
    struct sigaction sa;
    memset(&sa, 0, sizeof(sa));
    sa.sa_handler = sigusr_handler;
    sigemptyset(&sa.sa_mask);
    sa.sa_flags = 0;
    if(sigaction(SIGUSR1, &sa, NULL) == -1) {
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
    sigsuspend(&sigs);

    if(kill_signal_received){
      kill(-childpid,SIGTERM);
    }
    //PARENT
    /*
      Get CHILD PID
      Check if it is running, if not call for a restart
      If user requests stop, send kill signal,
      Reap orphans, force kill orphans maybe
    */

    close(spipe[1]);
  }
  return 0;
}




