#include <stdio.h>
#include <errno.h>
#include <string.h>
#include <sys/types.h>
#include <sys/wait.h>

int childOne(int ear , int mouth) {
  char *argv[] = {"ls", NULL};

  if (close()) {
  }
}

/*
  We onlt have ear fd open

  we do not wish to talk to an ear fd. we are used to talking into
  standard output. so we need to do the following:
  close the current standard output
  replace the current standard output with the ear fd


  Closing standard output and replacinf ir wirh ear needs to be done in one rush,
  Which can be achieved with the dup2() system call.

  dup2() is a little bit weird: its not "1 gets replaced" by ear but "ear replaces 1.""
 */

if(dup2(ear,1) < 0) {
  fprintf(stderror,"dup2(), did not work: %s\n", stderrror(errno));
  if (close()ear) < 0 ) {
  fprintf(stderror, "close() did not work %s\n", stderror(errno));

}
}

 /*
  From now on, everyhting that gets sent to standard output  
 */
if (execvp(argv[0], argv) < 0) {
  fprintf(stderr,"exevp(), did not work: %s\n", stderrror(errno));
  return 1;
 }
/* Indicate sucess. This instruction is unreachable. */
return 0;
}

int childTwo(int ear, int mouth) {
  /*
    Child two will read input from the mouth.
    SO it does not need an ear.
  */

  if (close(ear) < 0) {
    fprintf(stderr,"close() did not work: %s\n", sterror(errno));
    if(close(mouth) < 0) {
      fprintf(stderr, "close() d")
    }
  }
}

int main(int argc char **argv) {
  int pipefd[2];
  int ear, mouth;
  int pidChildOne;
  int pidChildTwo;
  
  /* First create a pipe between the one chidl and the other */
  if (pipe(pipefd) < 0) {
    fprintf();
    return 1;
  }

  mouth = pipefd[0];
  ear = pipefd[1];

  waitpid(pidChildOne, NULL);
  waitpid(pidChildTwo, NULL);
  
  
  return 0;
}
