#include <stdio.h>
#include <unistd.h>
#include <string.h>
#include <errno.h>
#include <sys/types.h>
#include <sys/wait.h>
#include <stdlib.h>

#define NAIVELOGINEXECUTABLE "naivelogin"

// Ochoa Alan
// 80639123
// Operating Systems
// Practical portion of midterm II

/* Wraps write(), calling it repeatedly until all count bytes have
   been written. 
*/
static ssize_t my_write(int fd, const void *buf, size_t count) {
  size_t to_be_written, already_written, written_this_time;
  ssize_t tmp;

  if (count == ((size_t) 0)) return (ssize_t) 0;
  
  for (to_be_written=count, already_written=(size_t) 0;
       to_be_written > (size_t) 0;
       to_be_written -= written_this_time, already_written += written_this_time) {
    tmp = write(fd, buf+already_written, to_be_written);
    if (tmp < ((ssize_t) 0)) return tmp;
    written_this_time = (size_t) tmp;
  }
  return (ssize_t) already_written;
}

/* Writes string str to fd, usign my_write() 

   Returns 0 for success.
   Returns -1 for error.

*/
static int write_string(int fd, const char *str) {
  size_t len;
  ssize_t res;

  len = strlen(str);
  if (len == ((size_t) 0)) return 0;
  res = my_write(fd, str, len);
  if (res < ((ssize_t) 0)) return -1;
  if (((size_t) res) != len) return -1;
  return 0;
}

/* Helper function for try_password().

   Closes the write end of the pipe.
   Dups the read end of the pipe to standard input.
   Replaces the executable by the one of 

   naivelogin,

   using execlp.

   Returns -1 on failure.
   Does not return or returns 0 on success.

*/
static int execute_child(int pipes[2]) {
  if (close(pipes[1]) < 0) {
    fprintf(stderr, "Cannot close a pipe end: %s\n", strerror(errno));
    if (close(pipes[0]) < 0) {
      fprintf(stderr, "Cannot close a pipe end: %s\n", strerror(errno));
    }
    return -1;
  }
  if (dup2(pipes[0], 0) < 0) {
    fprintf(stderr, "Cannot execute dup2(): %s\n", strerror(errno));
    if (close(pipes[0]) < 0) {
      fprintf(stderr, "Cannot close a pipe end: %s\n", strerror(errno));
    }
    return -1;
  }
  if (close(pipes[0]) < 0) {
    fprintf(stderr, "Cannot close a pipe end: %s\n", strerror(errno));
    return -1;
  }
  if (execlp(NAIVELOGINEXECUTABLE, NAIVELOGINEXECUTABLE, NULL) < 0) {
    fprintf(stderr, "Cannot replace process image: %s\n", strerror(errno));
    return -1;
  }
  /* Unreachable */
  return -1;
}

/* Spawns off a process running the executable

   naivelogin

   which must be in the PATH.

   Sends the string str to that process using a pipe. The process
   receives the string on its standard input.

   Tries to read remaining bytes off the pipe once the spawned process
   has died. Sets the variable pointed to by remainder to the amount
   of bytes read off the pipe after the death of the child process.

   Returns the exit code of the children process.

   If one of the system calls fails, returns -1 after printing an
   error message.

*/
static int try_password(size_t *remainder, const char *str) {
  int pipes[2];
  pid_t child;
  int wstatus, status;
  char buf[32];
  size_t bytes_read;
  ssize_t read_res;

  /* Set remainder to 0 */
  *remainder = (size_t) 0;
  
  /* Create a pipe */
  if (pipe(pipes) < 0) {
    fprintf(stderr, "Cannot create a pipe: %s\n", strerror(errno));
    return -1;
  }

  /* Fork off a child */
  child = fork();
  if (child < ((pid_t) 0)) {
    /* Fork did not work */
    fprintf(stderr, "Cannot fork: %s\n", strerror(errno));
    if (close(pipes[0]) < 0) {
      fprintf(stderr, "Cannot close a pipe end: %s\n", strerror(errno));
    }
    if (close(pipes[1]) < 0) {
      fprintf(stderr, "Cannot close a pipe end: %s\n", strerror(errno));
    }
    return -1;
  }
  if (child == ((pid_t) 0)) {
    /* Child */
    if (execute_child(pipes) < 0) exit(127);
    exit(0);
    /* Unreachable */
    return -1;
  }

  /* Parent */
  
  /* Send string down the pipe */
  if (write_string(pipes[1], str) < 0) {
    fprintf(stderr, "Cannot write to pipe: %s\n", strerror(errno));
    if (close(pipes[0]) < 0) {
      fprintf(stderr, "Cannot close a pipe end: %s\n", strerror(errno));
    }
    if (close(pipes[1]) < 0) {
      fprintf(stderr, "Cannot close a pipe end: %s\n", strerror(errno));
    }
    if (wait(NULL) < ((pid_t) 0)) {
      fprintf(stderr, "Cannot wait: %s\n", strerror(errno));
    }
    return -1;
  }  

  /* Wait for child */
  if (waitpid(child, &wstatus, 0) < 0) {
    fprintf(stderr, "Cannot wait: %s\n", strerror(errno));
    if (close(pipes[0]) < 0) {
      fprintf(stderr, "Cannot close a pipe end: %s\n", strerror(errno));
    }
    if (close(pipes[1]) < 0) {
      fprintf(stderr, "Cannot close a pipe end: %s\n", strerror(errno));
    }
    return -1;
  }
  status = (int) ((char) WEXITSTATUS(wstatus));

  /* If status is zero, return success, unless we cannot close the pipes */
  if (status == 0) {
    if (close(pipes[0]) < 0) {
      fprintf(stderr, "Cannot close a pipe end: %s\n", strerror(errno));
      if (close(pipes[1]) < 0) {
	fprintf(stderr, "Cannot close a pipe end: %s\n", strerror(errno));
      }
      return -1;
    }
    if (close(pipes[1]) < 0) {
      fprintf(stderr, "Cannot close a pipe end: %s\n", strerror(errno));
      return -1;
    }
    *remainder = (size_t) 0;
    return 0;
  }

  /* Close the write end of the pipe */
  if (close(pipes[1]) < 0) {
    fprintf(stderr, "Cannot close a pipe end: %s\n", strerror(errno));
    if (close(pipes[0]) < 0) {
      fprintf(stderr, "Cannot close a pipe end: %s\n", strerror(errno));
    }    
    return -1;
  }
    
  /* Try to read off as many bytes as possible from the read end of
     the pipe 
  */
  bytes_read = (size_t) 0;
  for (;;) {
    read_res = read(pipes[0], buf, sizeof(buf));
    if (read_res < ((ssize_t) 0)) {
      fprintf(stderr, "Cannot read: %s\n", strerror(errno));
      if (close(pipes[0]) < 0) {
	fprintf(stderr, "Cannot close a pipe end: %s\n", strerror(errno));
      }
      return -1;
    }
    if (read_res == ((ssize_t) 0)) break;
    bytes_read += (size_t) read_res;
  }

  /* Close read end of the pipe */
  if (close(pipes[0]) < 0) {
    fprintf(stderr, "Cannot close a pipe end: %s\n", strerror(errno));
    return -1;
  }

  /* Set remainder */
  *remainder = bytes_read;

  /* Return the status returned by the child process */
  return status;
}




/* Tries to crack the password compiled into 

   naivelogin

   Uses try_password()

   The password compiled into 

   naivelogin

   is 1 to 16 characters long.

   It is made out of the following characters

   abcdefghijklmnopqrstuvwxyzABCDEFGHIJKLMNOPQRSTUVWXYZ0123456789!@#$%&*_-+<>/

   Remark that the character '\n' is not among the possible characters
   for the password.

   If the password could be cracked, displays (on stdout)

   Cracked the password. The password is "<secret>".\n

   where <secret> is replaced by the recovered password.

   Returns 0 in this case.

   If an error occurs while running try_password(),
   displays (on stderr)

   An error occurred. Cannot crack the password.\n

   and returns 1.

   If the password could not be cracked by this
   program, displays (on stdout)

   Could not crack the password.\n

   and returns 1.

*/



// return 0 - pass return decrypted password
// return 1 - fail 
int main(int argc, char **argv) {
  char character_set[] = "abcdefghijklmnopqrstuvwxyzABCDEFGHIJKLMNOPQRSTUVWXYZ0123456789!@#$*_-+<>/"; // 75 available
  char password[] = "................"; // Since '.' not in character set fill as placeholder in 16 possible spots 
  
  size_t crack_cypher_attempt = ((size_t) 15); // remainder 
  size_t previous_attempt = ((size_t) 15);     // previous remainder 
  short  next_cypher_option = 0; // character_set ref 
  int tp_status; // type_password return value

  while ((tp_status = try_password(&crack_cypher_attempt, password)) != 0) {
    // Check try_password() return value 
    if (tp_status == -1) {
      fprintf(stderr,"An error occurred.Cannot crack the password.Error: %s \n", strerror(errno));
      return 1;
    }
   
    if (crack_cypher_attempt != previous_attempt) {
      previous_attempt = crack_cypher_attempt;
      next_cypher_option = 0; // reset back to 0 index in char_set 
    }
    // string comparison exploit  
    password[16-previous_attempt-1] = character_set[next_cypher_option++];
    // traversed through entire cypher options
    if (next_cypher_option > (strlen(character_set)-1)) {
      printf("Could not crack the password exhausted options\n");
      return 1;
    }
  }
  // remove unused spaces in geenerated password
  for (char *ptr = password; *ptr != '\0'; ptr++) {
    if (*ptr == '.') {
      *ptr = '\0';
    }
  }
  // success 
  printf("Cracked the password.The password is:%s\n", password);
  return 0;
}


