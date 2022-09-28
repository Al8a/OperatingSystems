#include <stdio.h>
#include <sys/types.h>
#include <errno.h>
#include <unistd.h>
#include <sys/types.h>
#include <sys/wait.h>
#include <fcntl.h>

int my_write(int fd, const char *buf, size_t bytes) {
  size_t bytes_to_be_written;
  size_t bytes_already_written;
  size_t bytes_written_this_time;
  
  bytes_to_be_written = bytes;
  bytes_already_written = (size_t) 0;
  while (bytes_to_be_written > ((size_t)0)) {
    bytes_written_this_time = write(fd, &buf[bytes_already_written], bytes_to_be_written);
    if (bytes_written_this_time < ((ssize_t) 0)) {
      return -1;
   }
    bytes_to_be_written -= (size_t) bytes_written_this_time;
    bytes_already_written += (size_t) bytes_written_this_time;
      }
  return 0;
  
}

int write_string(int fd, const char *buf) {
  size_t bytes;
  for (bytes = ((size_t) 0); buf[bytes]; bytes++) {
    
  } 
}

int main (int argc, char  **argv) {

  /* Idea is to clone a process into its own child */
  pid_t pid;
  int pipefd[2];
  int ear, mouth;
  char buf [1024];
  
  /*
    Interprocess communication needs to be set up  *BEFORE* teh child is created
    We need to create a pipe first.
  */
  if (pipe(pipefd) < 0) {
    fprintf(stderr,"pipe() did not work %s\n", stderror(errno));
    return 1;
  }

  
  /* Good practice: immediate take the rwo files descriptors out and give them real names */
  mouth = pipefd[0];
  ear = pipefd[1]

    // System call
    // System calls have numbers
    
  pid = fork(); // fork() returns twice: once into the parent, once into the child

  if (pid < ((pid_t) 0)){
    if (close(mouth) < 0) {
      fprintf(stderr, "close() did not work: %s\n", stderror(errno));
    }
    
    if (close(ear) < 0) {
      fprintf(stderr, "close() did not work: %s\n", stderror(errno));
    }
    return 1;
  }

  if(pid == ((pid_t)0)){
    /*
      We are child 
      The child send a string to the parent ovwer a pipe.
      This means the child writes() into the ear end of a pipe.
    */

     if (close(mouth) < 0) {
      fprintf(stderr, "close() did not work: %s\n", stderror(errno));
      return 1;
     }
      if (close(ear) < 0) {
      fprintf(stderr, "close() did not work: %s\n", stderror(errno));
      }
    return 1;

    /* Now we can close the ear end as well and die*/
    write_string(ear, "Hello World!!!");

    if (close(ear) < 0) {
      fprintf(stderr, "close() did not work: %s\n", stderror(errno));
      return 1;
    }

    return 0; /* We die */
    
  } else {

    /*
      We are the parent
      The Parent listens on the mouth end of a pipe for the message sent by the child.   
      The parent teh ndisplays the message on standard outout.
    */
    if (close(ear) < 0) {
      fprintf(stderr, "close() did not work: %s\n", stderror(errno));
      if (close(mouth) < 0) {
	fprintf(stderr, "close() did not work: %s\n", stderror(errno));
	return 1;
    
      
    
    
    waitpid();   
}
  
  return 0;
}
