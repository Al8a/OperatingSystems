#include <string.h>
#include <unistd.h>
#include <fcntl.h>
#include <stdio.h>  
#include <errno.h>  
#include "user_methods.h"

#define BUFFER_LEN (4096)

/* 
   Allowed systems calls:
   open()
   close()
   read()
   write()

   Possible Self_implementations:
   str_length()
   str_to_int()
   str_comp()
   mem_set()
   mem_cpy()
   mem_move()  
*/


int main(int argc, char **argv) {
  
  char buffer[BUFFER_LEN];
  char *file = NULL;
  int fd;
  int num;
  int n_lines_printed;

  ssize_t read_res; /* SIGNED SIZE */
  size_t remaining_bytes; 
  size_t bytes_written;
  size_t line_bytes;

  /* 
    10 Default number of lines read 
    unless -n argument specified in argv[] + filename specified  
  */
  num = 10;    
  for (int i = 1; i < argc; i++) {
    if (str_comp(argv[i],"-n") == 0) {   /* Check for -n arg */
      if (++i >= argc) {                 /* Determine if invalid input detected */
	display_error_message("head: option requires an argument -n [num]\n\tUsage: head -n <num> <filename>\n\tUsage: head <filename> -n <num>");
	return 1;
      }
      
      /* Convert input -n [num] to int */
      num = string_to_integer(argv[i]);
      if (num == 0) {   /* if zero, negative, or string entered */
        display_error_message("head: invalid num input\n");
        return 1;
      }
    } else {
      file = argv[i];
    }
  }

  fd = (file != NULL ? open(file, O_RDONLY) : 0); /* [statement] ? true : false  */

  /* Reading until file hits the end-of-file (EOF) condition */
  n_lines_printed = 0;

  while (n_lines_printed < num) {
    read_res = read(fd, buffer, sizeof(buffer)); /* Try to read into buffer  */

    /* Handle the return values of the read system call */
    /* Return value zero, indicates program is finished and at the end of the file (EOF) */
    if (read_res == ((ssize_t) 0)) break;

    /* Return value negative, error occured and we die */
    if (read_res < ((ssize_t) 0)) {
      /* Display the appropriate error message and die */
      display_error_message("Error reading! \n");
      return 1;
    }

    /* read_res assummed positive. */
    remaining_bytes = (size_t) read_res;

    /* If buffer filled but didn't reach end of the line */
    if ((remaining_bytes == sizeof(buffer)) && (buffer[remaining_bytes-1] != '\n')) {
      n_lines_printed--;
    }

    bytes_written = (size_t) 0;
    while ((remaining_bytes > (size_t) 0) && (n_lines_printed < num)) {
      line_bytes = get_line_bytes(&buffer[bytes_written], remaining_bytes);
      
      /* Need to write the remaining_bytes bytes from buffer to stdout */
      if (my_write(1, &buffer[bytes_written], line_bytes) < 0) {
        /* Display the appropriate error message and die */
        display_error_message("Error writing! \n");
        return 1;
      }
      remaining_bytes -= line_bytes;
      bytes_written += line_bytes;
      n_lines_printed++;
    }
  }
  
  if (fd != 0) {
    close(fd);
  }
  return 0;
}
