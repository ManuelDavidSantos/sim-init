#include <fcntl.h>
#include <stdio.h>
#include <stdlib.h>
#include <string.h>
#include <sys/types.h>
#include <sys/stat.h>
#include <sys/wait.h>
#include <unistd.h>

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

  pid_t mypid = fork();

  if(mypid < 0){
    perror("Failed to fork main process\n");
    exit(1);
  }
  if(mypid==0){
    //CHILD

    char dir_path[256]="/run/log/services/";
    strcat(dir_path, argv[2]);

    struct stat st = {0};
    if(stat(dir_path, &st) != -1){
      if(mkdir(dir_path,0700) == -1){
        perror("Failed to setup logging path\n");
        exit(1);
      }
    }
    
    char log_path[256];
    strcat(dir_path,log_path);
    strcat(log_path,"/log");

    int fd = open(log_path, O_APPEND | O_CREAT | O_WRONLY | O_CLOEXEC, S_IRWXU);

    close(spipe[0]);
    dup2(fd, STDOUT);
    for(int i = 0; i<10; i++){
      char * txt="Hiii\n";
      printf("%s",txt);
      fflush(stdout);
      sleep(1);
    }
  }
  if(mypid > 0){
    //PARENT
    close(spipe[1]);
    while(wait(NULL)>0);
  }
  return 0;
}
