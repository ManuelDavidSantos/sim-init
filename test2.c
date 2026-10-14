#include <stdio.h>
#include <stdlib.h>
#include <string.h>
#include <unistd.h>

int main(int argc, char *argv[], char * envp[])
{
  char text[200];
  char sst[2] = "ho";
  char st[9] = "aaaaaaaaa";
  strcpy(text,st);
  printf("%s\n",text);
  strcpy(text, sst);
  printf("%s\n",text);
  for(int i = 0; i<5; i++){
    usleep(100000);
    printf("hehehe");
    fflush(stdout);
  }
  return 0;
}
