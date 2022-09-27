#include <unistd.h>
#include <string.h> 
#include <stdlib.h>                         
#include <stdio.h>                         
#include <errno.h>                  
#include <fcntl.h>


int my_write(int fd, const char *buf, size_t bytes) {
  size_t bytes_to_be_written;
  size_t bytes_already_written;
  ssize_t bytes_written_this_time;

  bytes_to_be_written = bytes;
  bytes_already_written = (size_t) 0;
  
  while (bytes_to_be_written > ((size_t) 0)) {                            
    bytes_written_this_time = write(fd, &buf[bytes_already_written], bytes_to_be_written);
    if (bytes_written_this_time < ((ssize_t) 0)) {
      return -1;
    }           
    bytes_to_be_written -= (size_t) bytes_written_this_time;
    bytes_already_written += (size_t) bytes_written_this_time;
  }
  return 0;                                            
}


size_t get_line_bytes(char *buf, size_t num_bytes_remaining) {
  size_t n_bytes;
  n_bytes = (size_t) 0;

  while (buf[n_bytes] != '\n' && n_bytes < num_bytes_remaining) {
    n_bytes += sizeof(char);
  }
  
  if (buf[n_bytes] == '\n') {
    n_bytes += sizeof(char);
  }  
  return n_bytes;
}


/* Find string length */
int find_length(char *thatString){
  int length = 0;
  while(thatString[length] != '\0'){
    length++;
  }
  return length;
}
                                                                                                                     


int str_comp(const char *str1, const char *str2) {
  while (*str1 && (*str1 == *str2)) {  /* While *str1 is not NULL and equal to str2 */
    str1++;
    str2++;                                
  }                                        
  return *str1 - *str2; /* Return 0 if equal */                          
}                                          


int string_to_integer(char *str) {
    char c;
    int num = 0;
    for (c = *str; c >= '0' && c <= '9'; c = *++str) {
        num = num * 10 + (c - 48); /* ASCII value of '0' = 48 | '9' = 57 */ 
    }
    return num;
}

char *copy_str(char *inStr, size_t len) {
  char *out_str = (char*) malloc(sizeof(char) * (len + 1));
  char *copy = out_str;  
  for (int i = 0; i < len; i++) {
    *copy = *inStr;
    if(*copy == '\0') {
      break;
    }
    copy++;
    inStr++;
  }
  *copy = '\0';   /* set last index to '\0' */
  return out_str; /* Return new character array with null character at end appended */
}


/* Print to console */
int display_error_message(char *str){
  write(1, str, find_length(str));
  return 0;
}


