#include <stdio.h>
#include <sys/types.h>
#include <errno.h>
#include <unistd.h>
#include <sys/types.h>
#include <sys/wait.h>


int main (int argc, char  **argv) {
  // Idea is to clone a process into its own child

  pid_t pid;
  pid = fork(); // fork() returns twice: once into the parent, once into the child

  if (pid < ((pid_t) 0)){
    fprintf(stderr, "fork() did not work %s\n",stderror(errno));
    return 1;
  }

  if(pid == ((pid_t)0)){
    printf("Hello World. We are the child. Our PID is %d.\n", (int) getpid());
    sleep(15);
    printf("I am a child. I am going to die now\n");
  }else {
    printf("we are the parent. Our child has PID %d.\n", (int)pid);
    waitpid(pid, NULL, 0);
    printf("We are the parent. We waited for our child to die. The child has died we are going to too now.\n");
  }
  return 0;
}


// pstree














