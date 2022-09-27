#include <sys/types.h>
#include <sys/stat.h>
#include <sys/mman.h>
#include <stdlib.h>
#include <unistd.h>
#include <stddef.h>
#include <string.h>
#include <stdio.h>
#include <fcntl.h>
#include "user_methods.h"
/* 
   CS4375 OS Fall22 
   Homework 1 Part 4
   Due: 9/28/22 11:59PM 

   Sources: linuxhint.com, stackoverflow.com, geeksforgeeks,com 
*/


/* Define Structure of Data */
typedef struct entry_t {
  char prefix[6];
  char location[25];
  char newline;
  /* 6 + 25 + 1 = 32 character per line */
} entry_t;


/* Method to close file */
int closeFile(int fd){
  if (close(fd) < 0){
    display_error_message("Error closing file.\n");
    return 1;
  }
  return 0;
}

/* Do the actual comparison between given prefix and registry*/
int compare(const char *prefix_registry, const char *prefix_given){
  const char *reg;
  const char *giv;
  int i;

  for (reg = prefix_registry, giv = prefix_given, i = 0; i <= 5; i++, reg++, giv++){
    if (*reg < *giv) return -1;
    if (*reg > *giv) return 1;
  }
  
  return 0;
}

/* Lookup search the map for the given prefix */
char *lookup(entry_t registry[], ssize_t num_entries, const char *prefix){
  ssize_t l, r, m;
  int cmp;

  l = (ssize_t) 0;
  r = num_entries - ((ssize_t) 1);

  /* loop through registry to compare */
  while (l <= r) {
    m = ((l+r) / ((ssize_t) 2));
    cmp = compare(registry[m].prefix, prefix);
    if (cmp == 0) {
      return registry[m].location;
    } else if (cmp < 0) {
      l = m + ((ssize_t) 1);
    } else {
      r = m - ((ssize_t) 1);
    }
  }
  return NULL;
}



/* open the file and map the entries*/ 
int mapFile(const char *fileName, const char *prefix){
  /* Open file */
  int fd = open(fileName, O_RDONLY);
  if (fd < 0){
    display_error_message("Error opening file\n");
    return 1;
  }

  /* Find the end of the file in bytes */
  off_t lseek_res = lseek(fd, (off_t) 0, SEEK_END);
  if (lseek_res == (off_t) -1){
    display_error_message("Error seeking in file\n");
    closeFile(fd);
    return 1;
  }

  size_t fileSize = (size_t)lseek_res;
  if ((fileSize % sizeof(entry_t)) != (size_t) 0){
    display_error_message("Error, file is not properly formatted\n");
    closeFile(fd);
    return 1;
  }

  /* Map the file */
  char *ptr = NULL;
  ptr = mmap(NULL, fileSize, PROT_READ, MAP_SHARED, fd, (off_t) 0);
  
  
  if (ptr == MAP_FAILED){
    display_error_message("Error mapping file\n");
    closeFile(fd);
    return 1;
  }

  /* use the pointer to read the file */
  char *location; 
  location = lookup((entry_t*) ptr, ((ssize_t)(fileSize / sizeof(entry_t))), prefix);
  if (location == NULL) {
    display_error_message("That Prefix is not in the Registry\n");
  } else {
    //display_error_message(location);
    write(1, location, 25);
    display_error_message("\nThank you for using the Registry!\n\n");
  }

  /* unmapp */
  if (munmap(ptr, fileSize) != 0){
    display_error_message("Error unmapping file\n");
    closeFile(fd);
    return 1;
  }

  /* close file if everything works */
  closeFile(fd);
  return 0; 
}


int main(int argc, char **argv){
  /* check for valid inputs */
  if (argc < 3){
    display_error_message("Error: Lookup <filename> <prefix>\n");
    return 1;
  }
  
  /* assign the inputs to variables */
  char *fileName = argv[1];
  char *prefix = argv[2];
  
  /* display_error_message for the user */
  display_error_message("\nWelcome to the North American Prefix Registry\n");
  display_error_message("\tLook up in Progress...\n\n");

  /* call the methods to open and map files */
  mapFile(fileName, prefix);

  return 0;
}
