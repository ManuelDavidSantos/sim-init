#include <stdio.h>
#include <stdlib.h>

void count(){
  static int acc  = 0;
  printf("%d\n",acc++);
}

int main(int argc, char *argv[])
{
  count();
  count();
  count();
  return 0;
}
