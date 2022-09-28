#include <unistd.h>
#include <stdio.h>
#include <errno.h>
#include <string.h>

#define BUFFER_LEN (4096)

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

int main(int argc, char ** argv) {

    char buffer[BUFFER_LEN];
    ssize_t read_res;
    size_t read_bytes;
    /*We keep on reading until the file hits the end-of-line condition*/
    while (1) {
      /*If the returned vaalur is zero, we are done, as this is the end of line*/
      read_res = read(0, buffer, sizeof(buffer));
      if(read_res ==((ssize_t) 0)) break;
      if(read_res < ((ssize_t)0)) {
	fprintf(stderr, "Error reading: %s\n", strerror(errno)); 
	return 1;
      }
      /* Here we know that res is positive*/
      read_bytes = (size_t) read_res;

      /*We need to write all the read_btyes bytes from buffer to standard output*/
      if (my_write(1, buffer, read_bytes) < 0) {
	fprintf(stderr, "Error writing: %s\n", strerror(errno)); 
    }
  return 0;
}
}
