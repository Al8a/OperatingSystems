#ifndef _USER_METHODS_
#define _USER_METHODS_

/* 
    Calls write() until all bytes are written or until an error occurs. 
    Returns 0 on success
    Returns -1 on failure
*/
int my_write(int fd, const char *buf, size_t bytes);


/* Gets next buffer reading while checking against -n [num] remaining inputs */
size_t get_line_bytes(char *buf, size_t num_bytes_remaining);


/* Return the length of a given string */
size_t str_length(char *str);


/* 
    Returns the comparison of two strings
        return 1 : str1 > str2
        return 0 : str1 == str2
        return -1 : str1 < str2 
*/
int str_comp(const char *str1, const char *str2);


/* Converts read string input into integers using ASCII values */
int string_to_integer(char *str);


/* 
    Returns a freshly allocated new zero-terminated string
    containing <len> chars from <inStr> 
*/
char *copy_str(char *inStr, size_t len);


/* Print error message to console */
int display_error_message(char *str);
#endif
