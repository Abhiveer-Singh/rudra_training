#include <stdio.h>
int main(){

  #define PASS 
#ifndef PASS 
  printf("Debugging is enabled");
  return 0;
#endif
  printf("Program has terminated !!"); 
  return 0;
}
