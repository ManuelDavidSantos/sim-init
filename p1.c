#include <stdio.h>
#include <unistd.h>
int main(int argc, char * argv[]){
  printf("Initiating daemon...\n");
  int i = 0;
  while(1){
    sleep(10);
    printf("%i\n",i++);
  }
  return 0;
}
