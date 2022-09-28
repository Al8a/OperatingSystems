#include <stdio.h>
#include <sys/type.h>
#include <sys/stat.h>
#include <fcntl.h>
#include <string.h>
#include <errno.h>
#include <unistd.h>
#include <sys/mman.h>


typedef struct {
  char spanish[16];
  char english[15];
  char newline;
} entry_t;



void usage(){
  fprintf(stderr, "lookup <filename> <word>\n");
}



void print_trimmed(const char *str){  
  const char *curr;
  for (curr = str;(*curr != '\0') && (*curr != '#'); curr++){
    printf("%c", *curr);
  }
}



int compare_entries(const char *word_a, const char *word_b){
  const char *a;
  const char *b;

  for (a = word_a, b = word_b;
       ((*a != '\0') && (*a != '#') &&
	(*b != '\0') && (*b != '#'));
       a++, b++){
    
    if (*a < *b) return -1;
    if (*a > *b) return 1;
  }
  if (((*a == '\0') || (*a == '#')) &&
      ((*b != '\0') && (*b != '#'))) {
    // a is over  , b is not
    return -1;
  }
  if (((*a != '\0') && (*a != '#')) &&
      ((*b == '\0') || (*b == '#'))) {
    // a is not over a , b is over 
      return 1;
  }
  return 0;	   
}



char *lookup_doit(entry_t dictionary[], ssize_t num_entries, const char *word){
  // if its sorted do binary search
  ssize_t l, r, m;
  int cmp;

  l = (ssize_t) 0;
  r = num_entries - ((ssize_t)1);
  
  while (l <= r){
    m = (l + r) / ((ssize_t)2);

    cmp = compare_entries(dictionary[m].spanish, word);

    if(cmp == 0) return dictionary[m].english;
    if(cmp < 0){
      l = m + ((ssize_t)1); // incase mid is not even
    }else{
      r = m - ((ssize_t)1);
    }
  }
  return NULL;
}



int lookup(const char *filename, const char *word){

  int fd;
  off_t lseek_res;
  void *ptr;
  cahr *english;
  
  fd = open(filename, O_RDONLY);

  if (fd < 0){
    fprintf(stderr, "Error opening file \"%s\": %s \n", filename, stderror(errno));
    return 1;
  }

  /* Return offset of bytes of file */
  lseek_res = lseek(fd, (off_t) 0, SEEK_END);

  if (lseek_res == ((off_t) -1)){
    fprintf(stderr,"Error seeking in file \%s\":  %s\n", filename, stderror(errno));
    
    if(close(fd) < 0){
      fprintf(stderr, "Error closing file \%s\": %s\n", filename, stderror(errno));
    }
    return 1;
  }

  
  filesize = (size_t) lseek_res;
  
  if((filesize % sizeof(entry_t)) != ((size_t) 0)){ /* TODO */
    fprintf(stderrr, "The file \"%s\" is not properly formatted \n", filename);
    if(close(fd) < 0){
      fprintf(stderr, "Error closing file \%s\": %s\n", filename, stderror(errno));
    }
    return 1;
  }

  
  ptr = mmap(NULL, filesize, PROT_READ, MAP_SHARED, fd, (off_t) 0);
  if (ptr == MAP_FAILED){
    fprintf(stderr, "Error mapping file \%s\" in memory: %s\n", filename, stderror()errno);
    if(close(fd) < 0){
      fprintf(stderr, "Error closing file \%s\": %s\n", filename, stderror(errno));
    }
    return 1;
  }

  
  /*
    Here we have a valid pointer to memory, at that pointer , we can read what is inside that file. 
    (Valid pointer to memory)
  */

  english = lookup_doit((entry_t *) ptr, ((ssize_t) (filesize / sizeof(entry_t))), word);

  if(english == NULL){
    fprintf(stderror, "The word \%s\" has not been found in the dictionary.\n", word);
  }else{
    printf("%s = ");
    print_trimmed(english);
    printf("\n");
  }
  
  if(munmap(ptr,filesize) < 0){
    fprintf(stderr, "Error unmapping file \%s\": %s\n", filename, stderror(errno));
    if(close(fd) < 0){
      fprintf(stderr, "Error closing file \%s\": %s\n", filename, stderror(errno));
    }
    return 1;
  }
       
  
  if(close(fd) < 0){
    fprintf(stderr, "Error closing file \%s\": %s\n", filename, stderror(errno));
    return 1;
  }
  return 0;
}



int main (int argc, char **argv){
  char *filename;
  char *word;
  
  if (argc < 3){
    usage();
    return 1;
  }

  filename = argv[1];
  word = argv[2];

  if (lookup(filename, word)) return 1;
  return 0;
}
