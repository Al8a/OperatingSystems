#include <sys/types.h>
#include <sys/wait.h>
#include <sys/uio.h>
#include <unistd.h>
#include <string.h>
#include <fcntl.h>
#include <errno.h>
#include <stdio.h>
#include <stdlib.h>

/*--------------- 
  Ochoa, Alan
  80639123
----------------*/

/*
  Calls write() until all bytes are written or until an error occurs
  Return 0 on SUCCESS
  Return -1 on FAILURE
*/
int my_write(int fd, const char *buf, size_t bytes) {
  size_t bytes_to_be_written;
  size_t bytes_already_written;
  size_t bytes_written_this_time;
  
  bytes_to_be_written = bytes;
  bytes_already_written = (size_t) 0;
  
  while (bytes_to_be_written > ((size_t) 0)) {
    bytes_written_this_time = write(fd, &buf[bytes_already_written], bytes_to_be_written);
    if (bytes_written_this_time < ((size_t)0)) {
      return -1;
    }
    bytes_to_be_written -= (size_t) bytes_written_this_time;
    bytes_already_written += (size_t) bytes_written_this_time;
  }
  return 0;
}

/*
  File Descriptors:
    0 - standard input
    1 - standard output
    2 - standard error 
  ssize_t  write(int fd, const void *buf, size_t count) 
*/

/* Find string length */
int find_length(char *thatString){
  int length = 0;
  while(thatString[length] != '\0'){
    length++;
  }
  return length;
}

/* Print to console */
int display_error_message(char *str){
  write(2, str, find_length(str));
  return 0;
}

/* child process dup2() error checking */
int child(int ear, int mouth, int fd, char **argument){
	if(close(mouth) < 0){
		fprintf(stderr,"close() did not work: %s\n", strerror(errno));
		return 2;
	}
	if(close(fd) < 0){
		fprintf(stderr, "close() did not work: %s\n", strerror(errno));
		return 2;
	}
	
	/* Close standard output replace with the ear end of pipe */
	if(dup2(ear, 1) < 0) {
		fprintf(stderr, "dup2() failed: %s\n", strerror(errno));
		if(close(ear) < 0) {
			fprintf(stderr, "close() did not work: %s\n", strerror(errno));
		}
		return 2;
	}
	
	/* Close the ear/read end of the pipe */
	if(close(ear) < 0){
		fprintf(stderr, "close() did not work: %s\n", strerror(errno));
		return 2;
	}
	
	/* 
    Child process replaces itself with new executable
    name given by the 2nd command line argument argv[2].
  */
	if (execvp(argument[2], &argument[2]) < 0) {
		fprintf(stderr, "execvp() did not work: %s\n", strerror(errno));
		return 1;
	}
	return 0; /* SUCCESS */
}



int main(int argc, char **argv) {
  char buffer[4096];
  char *file;
  int fd;
  int ear, mouth;
  ssize_t read_res;
  size_t read_bytes;
  int pipefd[2];
  pid_t pid;


  /* Check arguments length */
  if (argc < 3) {
    //display_error_message("Not enough command line arguments.\n");
    fprintf(stderr,"Not enough command line arguments. %s\n", strerror(errno));
    return 1;
  }
  
  /* Open file to be used for writing */
  fd = open(argv[1], O_RDWR|O_CREAT , 0644);
  file = argv[1];
  if (fd < 0) {
    //display_error_message("Cannot open file \"%s\" %s: Permission denied\n");
    fprintf(stderr,"Cannot open file \"%s\" %s: Permission denied\n", file, strerror(errno));
    return 1;
  }

  /* Un-named pipe process */
  if (pipe(pipefd) < 0) {
    if (close(fd) < 0) {
      fprintf(stderr,"close() did not work %s\n", strerror(errno));
    }
    // display_error_message("Error using pipe()");
    fprintf(stderr,"pipe() did not work %s\n", strerror(errno));
    return 1;
  }

  mouth = pipefd[0]; /* read end of pipe */
  ear = pipefd[1];   /* write end of pipe */
  pid = fork();      /* Create fork process */

  /* fork() failed | no child process shall be created */
  if (pid < (0)) {
      fprintf(stderr, "fork() did not work: %s\n", strerror(errno));
      
      if (close(mouth) < 0) {
          fprintf(stderr, "close() did not work: %s\n", strerror(errno));
      }
      if (close(ear) < 0) {
          fprintf(stderr, "close() did not work: %s\n", strerror(errno));
      }
      if (close(fd) < 0) {
        fprintf(stderr, "close() did not work: %s\n", strerror(errno));
      }
    return 1;
  }


  /* fork() success | returns 0 to the child process */
  if (pid == (0)) {
	  return child(ear, mouth, fd, argv);	
	}
  

  /* read end of child process -> parent write */
  if (close(ear) < 0) {
    fprintf(stderr, "close() did not work: %s\n", strerror(errno));
  }

  /* Fill buffer w/ NULL terminator '\n' */
	for (int i = 0; i < sizeof(buffer); i++) {
		buffer[i] = '\0';
	}

	while(1) {
		/* Read from the mouth end of the pipe */
		read_res = read(mouth, buffer, sizeof(buffer)-1); 
		read_bytes = (size_t) read_res;
		
		if(read_res == ((ssize_t) 0)) break; /* End-of-file/Finished */
		
		/* Use argv[1] as writable file */
		if (my_write(fd, buffer, read_bytes) < 0) {
		  return 1;
		}
		
		/* Write to standard output */
		if (my_write(1, buffer, read_bytes) < 0) {
     	return 1;
		}
	}
	
	/* Wait on child to die */
	wait(NULL);
	
	/* Close file */
	if (close(fd) < 0) {
		fprintf(stderr, "close() did not work: %s\n", strerror(errno));
		return 1;
	}
  
  return 0; /* SUCCESS */
}