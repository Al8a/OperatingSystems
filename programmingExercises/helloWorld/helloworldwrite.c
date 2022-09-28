#include <unistd.h>

int main(int argc, char** argv){
  char str[] = "Hello World.\n";
  write(1, str, sizeof(str)); 
  
  return 0;
}
