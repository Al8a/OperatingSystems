#include <fcntl.h>
#include <string.h>
#include <stdlib.h>
#include <unistd.h>
#include <stdio.h>
#include <errno.h>
#include "user_methods.h"

#define BUFFER_LEN (4096)
#define FIRST_LINE_ALLOC_SIZE ((size_t) 2)
#define FIRST_ARRAY_SIZE ((size_t) 2)


int main(int argc, char **argv){
    char buffer[BUFFER_LEN];
    ssize_t read_res;  /* SIGNED */
    size_t read_bytes; /* UNSIGNED */

    int fd;
    int num;
    int temp;
    size_t i;
    char c;
    
    char *current_line;
    size_t current_line_len, current_line_size;

    char *ptr;
    char **lines;
    char **lptr;
    char *llptr;
    size_t lines_len, lines_size;
    char *lines_lengths;
    
    /* Read until file hits end-of-file condition */
    current_line_len = (size_t) 0;
    current_line_size = (size_t) 0;
    current_line = NULL;
    
    lines_len = (size_t) 0;
    lines_size = (size_t) 0;
    lines = NULL;
    lines_lengths = NULL;


    
    while(1){
        /* Try to read into the buffer 
	   sizeof(buffer) bytes 
	*/
        read_res = read(0, buffer, sizeof(buffer));
        
        /* Handle the return values of the read system call */
        
        /* Return value zero, done, end-of-file reached */
        if (read_res == ((ssize_t) 0)) break;
        
        /* Return value negative, we have an error and we die */
        if (read_res < ((ssize_t) 0)) {
	  /* Display the appropriate error message and die */
	  display_error_message( "Error reading: %s\n");
	  /* Deallocate everything that has been allocated */
	  for (i=(size_t) 0; i<lines_len; i++){
	    free(lines[i]);
	  }
	  free(lines);
	  free(lines_lengths);
	  return 1;
	}
        
        /* read_res is positive */
        read_bytes = (size_t) read_res;
        
        /* Handle input and store into memory */
        for(i=(size_t) 0; i < read_bytes; i++){
	  /* Get current character */
	  c = buffer[i];
            
	  /* Put the current character into the current line */
	  if ((current_line_len + ((size_t) 1)) > current_line_size){
	    /* Allocate Memory */
	    if ((current_line_len == ((size_t) 0)) ||
		(current_line == NULL)) {
	      /* First allocation */
	      ptr = (char *) malloc(FIRST_LINE_ALLOC_SIZE * sizeof(char)); //Each char is 1 byte
	      if (ptr == NULL) {
		display_error_message("Could not allocate any more memory.\n");
		/* Deallocate everything that has been allocated */
		for (i=(size_t) 0; i<lines_len; i++){
		  free(lines[i]);
		}
		free(lines);
		free(lines_lengths);
		return 1;
	      }
	      current_line = ptr;
	      current_line_size = FIRST_LINE_ALLOC_SIZE;
	    } else {

	      /* Preform Reallocation */
	      ptr = (char *) realloc(current_line, current_line_size * ((size_t) 2) * sizeof(char));

	      if (ptr == NULL){
		display_error_message("Could not allocate any more memory.\n");
		/* Deallocate everything that has been allocated */
		for (i = (size_t) 0; i < lines_len; i++) {
		  free(lines[i]);
		}
		free(lines);
		free(lines_lengths);
		return 1;
	      }
	      current_line = ptr;
	      current_line_size *= (size_t) 2;
	    }
	  }
            
	  /*
	    Here, we are sure to have the right space in memory. 
	     We put the character in and increment the length
	  */
	  current_line[current_line_len] = c;
	  current_line_len++;
            
            
	  /* If this is a newline character, start a new line */
	  if (c == '\n'){
	    if ((lines_len + ((size_t) 1)) > lines_size) {
	      /* Allocate memory :) */
	      if ((lines_len == ((size_t) 0)) ||
		  (lines == NULL)) {
		/* First allocation */
		lptr = (char **) malloc(FIRST_ARRAY_SIZE * sizeof(char *));
		if(lptr == NULL){
		  display_error_message("Could not allocate any more memory.\n");
		  /* Deallocate everything that has been allocated */
		  for (i=(size_t) 0; i<lines_len; i++){
		    free(lines[i]);
		  }
		  free(lines);
		  free(lines_lengths);
		  return 1;
		}
		lines = lptr;
		lines_size = FIRST_ARRAY_SIZE;
		llptr = (char *) malloc(FIRST_ARRAY_SIZE * sizeof(char));
		if(llptr == NULL){
		  display_error_message("Could not allocate any more memory.\n");
		  /* Deallocate everything that has been allocated */
		  for (i=(size_t) 0; i<lines_len; i++){
		    free(lines[i]);
		  }
		  free(lines);
		  free(lines_lengths);
		  return 1;
		}
		lines_lengths = llptr;
	      } else {
		/* Reallocation */
		lptr = (char **) realloc(lines, lines_size * ((size_t) 2) * sizeof(char *));
                
		if(lptr == NULL){
		  display_error_message("Could not allocate any more memory.\n");
		  /* Deallocate everything that has been allocated */
		  for (i=(size_t) 0; i<lines_len; i++){
		    free(lines[i]);
		  }
		  free(lines);
		  free(lines_lengths);
		  return 1;
		}
		lines = lptr;
		lines_size *= (size_t) 2;
                
		/* Reallocation */
		llptr = (char *) realloc(lines_lengths, lines_size * ((size_t) 2) * sizeof(char));
                
		if(llptr == NULL){
		  display_error_message("Could not allocate any more memory.\n");
		  /* Deallocate everything that has been allocated */
		  for (i=(size_t) 0; i<lines_len; i++){
		    free(lines[i]);
		  }
		  free(lines);
		  free(lines_lengths);
		  return 1;
		}
		lines_lengths = llptr;
	      }
	    }
	    lines[lines_len] = current_line;
	    lines_lengths[lines_len] = current_line_len;
	    lines_len++;
	    current_line = NULL;
	    current_line_len = (size_t) 0;
	    current_line_size = (size_t) 0;
	  }
        }
        
    }
    
    /*
      In the case when the last line has no new line character at the
      end we need to put the line into the array of lines nevertheless 
    */
    if ( !((current_line == NULL) || (current_line_len == (size_t) 0)) ) {
      if ((lines_len + ((size_t) 1)) > lines_size) {
	/* Allocate memory :) */
	if ((lines_len == ((size_t) 0)) ||
            (lines == NULL)) {
	  /* First allocation */
	  lptr = (char **) malloc(FIRST_ARRAY_SIZE * sizeof(char *));
	  if(lptr == NULL){
	    display_error_message("Could not allocate any more memory.\n");
	    /* Deallocate everything that has been allocated */
	    for (i=(size_t) 0; i<lines_len; i++){
	      free(lines[i]);
	    }
	    free(lines);
	    free(lines_lengths);
	    return 1;
	  }
	  lines = lptr;
	  lines_size = FIRST_ARRAY_SIZE;
	  llptr = (char *) malloc(FIRST_ARRAY_SIZE * sizeof(char));
	  if(llptr == NULL){
	    display_error_message("Could not allocate any more memory.\n");
	    /* Deallocate everything that has been allocated */
	    for (i=(size_t) 0; i<lines_len; i++){
	      free(lines[i]);
	    }
	    free(lines);
	    free(lines_lengths);
	    return 1;
	  }
	  lines_lengths = llptr;
	} else {
	  /* Reallocation */
	  lptr = (char **) realloc(lines, lines_size * ((size_t) 2) * sizeof(char *));
	  
	  if(lptr == NULL){
	    display_error_message("Could not allocate any more memory.\n");
	    /* Deallocate everything that has been allocated */
	    for (i=(size_t) 0; i<lines_len; i++){
	      free(lines[i]);
	    }
	    free(lines);
	    free(lines_lengths);
	    return 1;
	  }
	  lines = lptr;
	  lines_size *= (size_t) 2;
	  
	  /* Reallocation */
	  llptr = (char *) realloc(lines_lengths, lines_size * ((size_t) 2) * sizeof(char));
          
	  if(llptr == NULL){
	    display_error_message("Could not allocate any more memory.\n");
	    /* Deallocate everything that has been allocated */
	    for (i=(size_t) 0; i < lines_len; i++){
	      free(lines[i]);
	    }
	    free(lines);
	    free(lines_lengths);
	    return 1;
	  }
	  lines_lengths = llptr;
	}
        
	lines[lines_len] = current_line;
	lines_lengths[lines_len] = current_line_len;
	lines_len++;
	current_line = NULL;
	current_line_len = (size_t) 0;
	current_line_size = (size_t) 0;
      }
    }
    
    /*
      Here, we have an array lines of "strings", i.e. array lines of 
      pointers to characters. There are lines_len set by [num] valid entries
    */
    
    temp = 0;
    num = 10;
    for (i= lines_len; i > (size_t) 0; i--){
      if (temp < num) {
	my_write(1, lines[i - ((size_t)1)], lines_lengths[i - ((size_t) 1)]);
	temp++;
      } else {
	break;
      }
    }
    
    /* Deallocate everything that has been allocated */
    for (i=(size_t) 0; i<lines_len; i++){
      free(lines[i]);
    }
    free(lines);
    free(lines_lengths);
    
    return 0; /* Signal success */
}
