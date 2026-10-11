#include <errno.h>
#include <fcntl.h>
#include <stdio.h>
#include <stdlib.h>
#include <string.h>
#include <sys/types.h>
#include <sys/stat.h>
#include <sys/wait.h>
#include <sys/prctl.h>
#include <unistd.h>
#include "stddef.h"

#define STDOUT 1

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


  if (prctl(PR_SET_CHILD_SUBREAPER, 1) == -1) {
    perror("prctl(PR_SET_CHILD_SUBREAPER)");
  }
  
  pid_t mypid = fork();

  if(mypid < 0){
    perror("Failed to fork main process\n");
    exit(1);
  }
  if(mypid==0){
    //CHILD

    char dir_path[256]="/home/voidy/services/";
    strcat(dir_path, argv[2]);

    struct stat st = {0};
    if(stat(dir_path, &st) == -1){
      if(mkdir(dir_path,0700) == -1){
        fprintf(stderr, "mkdir(%s) failed: %s\n", dir_path, strerror(errno));
        exit(1);
      }
    }    
    strcat(dir_path,"/log");
    
    //printf("Path:%s\n",dir_path);

    int fd = open(dir_path, O_APPEND | O_CREAT | O_WRONLY | O_CLOEXEC, 0644);

    close(spipe[0]);
    dup2(fd, STDOUT);
    char * args[] = {
      argv[1],
      NULL
    }; 

    execvp(argv[1], args);

  }
  if(mypid > 0){
    int childpid = mypid;

    //PARENT
    /*
      Get CHILD PID
      Check if it is running, if not call for a restart
      If user requests stop, send kill signal,
      Reap orphans, force kill orphans maybe
    */

    close(spipe[1]);
    while(wait(NULL));
  }
  return 0;
}




