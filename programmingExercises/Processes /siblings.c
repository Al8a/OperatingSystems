#include <stdio.h>
#include <sys/types.h>
#include <errno.h>
#include <unistd.h>
#include <sys/types.h>
#include <sys/wait.h>

void child() {

}


int main (int argc, char  **argv) {
  // Dont do this more process than expected
  /*
  pid_t child1, child2, child3;
  child1 = fork();
  child2 = fork();
  child3 = fork();
  */

  for (int i = 0; i < 5; i++) {
    pid_t pid = fork();
    if(pid == 0) {
      // I am child
      child();
      break;
    }
    
  }
  return 0;
}
