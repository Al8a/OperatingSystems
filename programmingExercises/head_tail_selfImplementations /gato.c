#include <unistd.h>
#include <errno.h>
#include <stdio.h>
#include <string.h>

#define BUFFER_LEN (4096)

/* Calls write() until all bytes are written or until an error occurs
   returns 0 on success
   returns -1 on failure  
 */
  
int my_write(int fd, const char *buf, size_t bytes){
  size_t bytes_to_be_written;
  size_t bytes_already_written;
  ssize_t bytes_written_this_time;
  
  bytes_to_be_written = bytes;
  bytes_already_written = (size_t) 0;

  while(bytes_to_be_written > ((size_t)0)){
    bytes_written_this_time = write(fd, &buf[bytes_already_written], bytes_to_be_written);

    if(bytes_written_this_time < ((ssize_t) 0)){
      return -1;  
    }

    bytes_to_be_written -= (size_t) bytes_written_this_time;
    bytes_already_written += (size_t) bytes_written_this_time; // check for overflow 
  }
  return 0;
}


int main(int argc, char ** argv){ 
  char buffer[BUFFER_LEN];
  ssize_t read_res;  // signed size
  size_t read_bytes; // unsigned size
  
  // Keep reading until end-of-file condition met
  while(1){

    // try to read into the buffer, up to sizeof(buffer) bytes
    read_res = read(0, buffer, sizeof(buffer));

    /* Handle the return values of the read system call */
    /* -------------------------------------------------------*/

    // if returned value is 0, we are done and hit end-of-file 
    if(read_res == ((ssize_t)0)) break;

    // if returned value is negative, we have an error and we die
    if(read_res < ((ssize_t) 0)) {
      // Display the appropriate error message and die
      fprintf(stderr, "Error reading: %s\n", strerror(errno)); 
      return 1;
    }

    // Here we know that read_res is positive
    read_bytes = (size_t) read_res;

    // We need to write all the read_bytes bytes from buffer to standard output
    if(my_write(1,buffer, read_bytes) < 0){
      fprintf(stderr, "Error reading: %s\n", strerror(errno)); 
      return 1;
    }
  } 
  return 0;
}
