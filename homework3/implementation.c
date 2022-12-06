/*
  MyFS: a tiny file-system written for educational purposes

  MyFS is 

  Copyright 2018-21 by

  University of Alaska Anchorage, College of Engineering.

  Copyright 2022

  University of Texas at El Paso, Department of Computer Science.

  Contributors: Christoph Lauter,
                Alan Ochoa 
                Roxxanne White and
                Natasha Rovelli


  and based on 

  FUSE: Filesystem in Userspace
  Copyright (C) 2001-2007  Miklos Szeredi <miklos@szeredi.hu>

  This program can be distributed under the terms of the GNU GPL.
  See the file COPYING.

  gcc -Wall myfs.c implementation.c `pkg-config fuse --cflags --libs` -o myfs

*/

#include <stddef.h>
#include <sys/stat.h>
#include <sys/statvfs.h>
#include <stdint.h>
#include <string.h>
#include <time.h>
#include <stdlib.h>
#include <sys/types.h>
#include <unistd.h>
#include <fcntl.h>
#include <errno.h>
#include <stdio.h>
// #include "implementation.h"


/* The filesystem you implement must support all the 13 operations
   stubbed out below. There need not be support for access rights,
   links, symbolic links. There needs to be support for access and
   modification times and information for statfs.

   The filesystem must run in memory, using the memory of size 
   fssize pointed to by fsptr. The memory comes from mmap and 
   is backed with a file if a backup-file is indicated. When
   the filesystem is unmounted, the memory is written back to 
   that backup-file. When the filesystem is mounted again from
   the backup-file, the same memory appears at the newly mapped
   in virtual address. The filesystem datastructures hence must not
   store any pointer directly to the memory pointed to by fsptr; it
   must rather store offsets from the beginning of the memory region.

   When a filesystem is mounted for the first time, the whole memory
   region of size fssize pointed to by fsptr reads as zero-bytes. When
   a backup-file is used and the filesystem is mounted again, certain
   parts of the memory, which have previously been written, may read
   as non-zero bytes. The size of the memory region is at least 2048
   bytes.

   CAUTION:

   * You MUST NOT use any global variables in your program for reasons
   due to the way FUSE is designed.

   You can find ways to store a structure containing all "global" data
   at the start of the memory region representing the filesystem.

   * You MUST NOT store (the value of) pointers into the memory region
   that represents the filesystem. Pointers are virtual memory
   addresses and these addresses are ephemeral. Everything will seem
   okay UNTIL you remount the filesystem again.

   You may store offsets/indices (of type size_t) into the
   filesystem. These offsets/indices are like pointers: instead of
   storing the pointer, you store how far it is away from the start of
   the memory region. You may want to define a type for your offsets
   and to write two functions that can convert from pointers to
   offsets and vice versa.

   * You may use any function out of libc for your filesystem,
   including (but not limited to) malloc, calloc, free, strdup,
   strlen, strncpy, strchr, strrchr, memset, memcpy. However, your
   filesystem MUST NOT depend on memory outside of the filesystem
   memory region. Only this part of the virtual memory address space
   gets saved into the backup-file. As a matter of course, your FUSE
   process, which implements the filesystem, MUST NOT leak memory: be
   careful in particular not to leak tiny amounts of memory that
   accumulate over time. In a working setup, a FUSE process is
   supposed to run for a long time!

   It is possible to check for memory leaks by running the FUSE
   process inside valgrind:

   valgrind --leak-check=full ./myfs --backupfile=test.myfs ~/fuse-mnt/ -f

   However, the analysis of the leak indications displayed by valgrind
   is difficult as libfuse contains some small memory leaks (which do
   not accumulate over time). We cannot (easily) fix these memory
   leaks inside libfuse.

   * Avoid putting debug messages into the code. You may use fprintf
   for debugging purposes but they should all go away in the final
   version of the code. Using gdb is more professional, though.

   * You MUST NOT fail with exit(1) in case of an error. All the
   functions you have to implement have ways to indicated failure
   cases. Use these, mapping your internal errors intelligently onto
   the POSIX error conditions.

   * And of course: your code MUST NOT SEGFAULT!

   It is reasonable to proceed in the following order:

   (1)   Design and implement a mechanism that initializes a filesystem
         whenever the memory space is fresh. That mechanism can be
         implemented in the form of a filesystem handle into which the
         filesystem raw memory pointer and sizes are translated.
         Check that the filesystem does not get reinitialized at mount
         time if you initialized it once and unmounted it but that all
         pieces of information (in the handle) get read back correctly
         from the backup-file. 

   (2)   Design and implement functions to find and allocate free memory
         regions inside the filesystem memory space. There need to be 
         functions to free these regions again, too. Any "global" variable
         goes into the handle structure the mechanism designed at step (1) 
         provides.

   (3)   Carefully design a data structure able to represent all the
         pieces of information that are needed for files and
         (sub-)directories.  You need to store the location of the
         root directory in a "global" variable that, again, goes into the 
         handle designed at step (1).
          
   (4)   Write __myfs_getattr_implem and debug it thoroughly, as best as
         you can with a filesystem that is reduced to one
         function. Writing this function will make you write helper
         functions to traverse paths, following the appropriate
         subdirectories inside the file system. Strive for modularity for
         these filesystem traversal functions.

   (5)   Design and implement __myfs_readdir_implem. You cannot test it
         besides by listing your root directory with ls -la and looking
         at the date of last access/modification of the directory (.). 
         Be sure to understand the signature of that function and use
         caution not to provoke segfaults nor to leak memory.

   (6)   Design and implement __myfs_mknod_implem. You can now touch files 
         with 

         touch foo

         and check that they start to exist (with the appropriate
         access/modification times) with ls -la.

   (7)   Design and implement __myfs_mkdir_implem. Test as above.

   (8)   Design and implement __myfs_truncate_implem. You can now 
         create files filled with zeros:

         truncate -s 1024 foo

   (9)   Design and implement __myfs_statfs_implem. Test by running
         df before and after the truncation of a file to various lengths. 
         The free "disk" space must change accordingly.

   (10)  Design, implement and test __myfs_utimens_implem. You can now 
         touch files at different dates (in the past, in the future).

   (11)  Design and implement __myfs_open_implem. The function can 
         only be tested once __myfs_read_implem and __myfs_write_implem are
         implemented.

   (12)  Design, implement and test __myfs_read_implem and
         __myfs_write_implem. You can now write to files and read the data 
         back:

         echo "Hello world" > foo
         echo "Hallo ihr da" >> foo
         cat foo

         Be sure to test the case when you unmount and remount the
         filesystem: the files must still be there, contain the same
         information and have the same access and/or modification
         times.

   (13)  Design, implement and test __myfs_unlink_implem. You can now
         remove files.

   (14)  Design, implement and test __myfs_unlink_implem. You can now
         remove directories.

   (15)  Design, implement and test __myfs_rename_implem. This function
         is extremely complicated to implement. Be sure to cover all 
         cases that are documented in man 2 rename. The case when the 
         new path exists already is really hard to implement. Be sure to 
         never leave the filessystem in a bad state! Test thoroughly 
         using mv on (filled and empty) directories and files onto 
         inexistant and already existing directories and files.

   (16)  Design, implement and test any function that your instructor
         might have left out from this list. There are 13 functions 
         __myfs_XXX_implem you have to write.

   (17)  Go over all functions again, testing them one-by-one, trying
         to exercise all special conditions (error conditions): set
         breakpoints in gdb and use a sequence of bash commands inside
         your mounted filesystem to trigger these special cases. Be
         sure to cover all funny cases that arise when the filesystem
         is full but files are supposed to get written to or truncated
         to longer length. There must not be any segfault; the user
         space program using your filesystem just has to report an
         error. Also be sure to unmount and remount your filesystem,
         in order to be sure that it contents do not change by
         unmounting and remounting. Try to mount two of your
         filesystems at different places and copy and move (rename!)
         (heavy) files (your favorite movie or song, an image of a cat
         etc.) from one mount-point to the other. None of the two FUSE
         processes must provoke errors. Find ways to test the case
         when files have holes as the process that wrote them seeked
         beyond the end of the file several times. Your filesystem must
         support these operations at least by making the holes explicit 
         zeros (use dd to test this aspect).

   (18)  Run some heavy testing: copy your favorite movie into your
         filesystem and try to watch it out of the filesystem.

*/

/* YOUR HELPER FUNCTIONS GO HERE */

#include <stddef.h>
#include <sys/stat.h>
#include <sys/statvfs.h>
#include <stdint.h>
#include <string.h>
#include <time.h>
#include <stdlib.h>
#include <sys/types.h>
#include <unistd.h>
#include <fcntl.h>
#include <errno.h>
#include <stdio.h>


#define MYFS_MAXIMUM_NAME_LENGTH (256)
#define MYFS_size ((size_t) 1024)
//#define MYFS_size ((size_t) (4096))
#define MYFS_STATIC_PATH_BUF_SIZE (8192)
#define MYFS_TRUNCATE_SMALL_ALLOCATE ((size_t) 512)
#define MYFS_MAGIC ((uint32_t) (UINT32_C(0xcafebabe)))


typedef size_t __myfs_offset_t; 
typedef unsigned int u_int;
typedef unsigned long u_long;

/* Structs Declarations -------------------- */

/*  Handler structure "Super-block" */ 
typedef struct __myfs_handle_struct_t __myfs_handle_t;
struct __myfs_handle_struct_t {
  uint32_t magic;
  __myfs_offset_t free_memory;
  __myfs_offset_t root_directory;
  size_t size;
};


/* Memory block structure */
typedef struct __myfs_memory_block_struct_t __myfs_mem_block_t;
struct __myfs_memory_block_struct_t {
  size_t size;
  size_t user_size;
  __myfs_offset_t next; 
};



/* File Block Structure */ 
typedef struct __myfs_file_block_struct_t {
  size_t size;
  size_t allocated;
  __myfs_offset_t next;
  __myfs_offset_t data;
} __myfs_file_block_t;



typedef enum __myfs_inode_enum_type {
  DIRECTORY,
  REG_FILE
} __myfs_inode_type_t;

typedef struct __myfs_inode_struct_file_t {
  size_t size; 
  __myfs_offset_t first_block;
} __myfs_inode_file_t;


typedef struct __myfs_inode_struct_directory_t{
  size_t number_children;
  __myfs_offset_t children;  
} __myfs_inode_directory_t;


/* inode struct */
typedef struct __myfs_inode_struct_t __myfs_inode_t;
struct __myfs_inode_struct_t {
  char name[MYFS_MAXIMUM_NAME_LENGTH];
  struct timespec accessed_time;
  struct timespec modified_time;
  __myfs_inode_type_t type;
  union {
      __myfs_inode_file_t file;  
      __myfs_inode_directory_t directory;
    } value;
};


static inline __myfs_offset_t ptr_to_offset(void *ptr, void *fstpr){
  if (ptr < fstpr) return 0;
  return (__myfs_offset_t) (ptr - fstpr);
}


static inline void *offset_to_ptr(void *ptr, __myfs_offset_t offset){
  if (offset == 0) return NULL;
  return (void *) (ptr + offset);
}



__myfs_handle_t *__myfs_get_handle(void *fsptr, size_t size){
  __myfs_handle_t *handle = (__myfs_handle_t *) fsptr;
  __myfs_mem_block_t *block;
  size_t s;
  if (size < sizeof(struct __myfs_handle_struct_t)) return NULL;

  if (handle->magic != MYFS_MAGIC) {
    s = (size - (sizeof(struct __myfs_handle_struct_t)));  
    if (handle->magic != ((uint32_t) 0)) {
      memset((fsptr + sizeof(struct __myfs_handle_struct_t)), 0, s);
    }
    handle->magic = MYFS_MAGIC; 
    handle->size = s;
	
    if (s == ((size_t) 0)) {
      handle->free_memory = ((__myfs_offset_t) 0);
	  
    } else {
      block = (__myfs_mem_block_t *) offset_to_ptr(fsptr, sizeof(handle));
      block->size = s;
      block->next = (__myfs_offset_t) 0;
      handle->free_memory = ptr_to_offset(fsptr,block);
    }           
    handle->root_directory = (__myfs_offset_t) 0;
  }
  
  return handle;
}



size_t free_memory_size(__myfs_handle_t *handle) {
  size_t total_free_size;
  __myfs_mem_block_t *block;
  
  total_free_size = (size_t) 0;
  for (block = (__myfs_mem_block_t *) offset_to_ptr(handle, handle->free_memory);
       block != NULL; block = (__myfs_mem_block_t *) (offset_to_ptr(handle,block->next))) {
      total_free_size += block->size; 
    }
    return total_free_size;
}



__myfs_mem_block_t *get_memory_block(__myfs_handle_t *handle, size_t size){
  __myfs_mem_block_t *curr, *prev, *next;
  for (curr = (__myfs_mem_block_t *) offset_to_ptr(handle, handle->free_memory),
	 prev = NULL; curr != NULL && curr->next != (__myfs_offset_t) 0; prev = curr,
	 curr = (__myfs_mem_block_t *) offset_to_ptr(handle, curr->next)) { 
    if (curr->size > size) {
      break;
    }
  }
    
  if (curr->size < size) {
    return NULL;
  }
    
  if (curr->size - size > (size_t) 0) { 
      next = (__myfs_mem_block_t *) offset_to_ptr(curr, size);
      next->size = curr->size - size;
      next->next = curr->next;
  } else { 
    next = (__myfs_mem_block_t *) offset_to_ptr(handle, curr->next);
  }
  
  // curr first available memory block
  if (curr == (__myfs_mem_block_t *) offset_to_ptr(handle, handle->free_memory)) {
    handle->free_memory = ptr_to_offset(next, handle);
  } else {
    prev->next = ptr_to_offset(next, handle);
  }
  curr->size = size; 
  curr->next = (__myfs_offset_t) 0;
  
  return curr;
}



// add block back to free memory linked list
void add_to_free_memory(__myfs_handle_t *handle, __myfs_offset_t offset) {
  __myfs_mem_block_t *mem_block, *curr, *prev;
  mem_block = (__myfs_mem_block_t *) offset_to_ptr(handle, offset);
  for (curr = (__myfs_mem_block_t *) offset_to_ptr(handle, handle->free_memory), 
         prev = NULL; curr != NULL; prev = curr,
         curr = (__myfs_mem_block_t *) offset_to_ptr(handle, curr->next)) {
    
    if ((void *) mem_block < (void *) curr) {
      break;
    }
  }

  // place block in between prev and curr block
  if (curr != NULL) {
    mem_block->next = ptr_to_offset(curr, handle);
  } else {
    mem_block->next = (__myfs_offset_t) 0;
  }
  
  
  if (prev == NULL){
    handle->free_memory = offset;      
  } else {
    prev->next = offset;
  }
  
  if (curr != NULL && ((void *) ((void *) mem_block + mem_block->size)) == ((void *) curr)) {
    mem_block->size += curr->size;
    mem_block->next = curr->next;
  }
  
  if (prev != NULL && ((void *) ((void *) prev + prev->size)) == ((void *) mem_block)) {
      prev->size += mem_block->size;
      prev->next = mem_block->next;
  }
  
    return;
}


void __myfs_free_impl(__myfs_handle_t *handle, __myfs_offset_t offset) {
  void *ptr;
  ptr = (((void *) offset_to_ptr(handle, offset)) - ((size_t) sizeof(__myfs_mem_block_t))); 
  __myfs_offset_t new_offset = ptr_to_offset(ptr, handle);
  add_to_free_memory(handle, new_offset);
}



__myfs_offset_t __myfs_allocate_memory(__myfs_handle_t *handle, size_t size) {
  size_t s, offset_mem;
  void *ptr;
  
  if (size == ((size_t) 0)) { 
    return (__myfs_offset_t) 0;
  }
  
  s = (size + (size_t) sizeof(__myfs_mem_block_t));
  if (s < size) return (__myfs_offset_t) 0;
  
  ptr = ((void *) get_memory_block(handle, s));
  
  if (ptr != NULL) {
    return  ptr_to_offset((ptr + (size_t) sizeof(__myfs_mem_block_t)), handle);
  }
  return (__myfs_offset_t) 0;
}


__myfs_offset_t __myfs_reallocate_memory(__myfs_handle_t *handle, __myfs_offset_t offset, size_t size) {
    __myfs_mem_block_t *old_mem_block;
    __myfs_offset_t new_offset;
    void *old_ptr, *new_mem_block;
    size_t s;
    
    if (handle == NULL) return (__myfs_offset_t) 0;
    if (offset == (__myfs_offset_t) 0) return (__myfs_offset_t) 0;
    if (size == (size_t) 0) {
        __myfs_free_impl(handle, offset);
        return (__myfs_offset_t) 0;
    }

    new_offset = __myfs_allocate_memory(handle, size);
    if (new_offset == (__myfs_offset_t) 0) return (__myfs_offset_t) 0;  

    old_ptr = offset_to_ptr(handle, offset);
    old_mem_block = (__myfs_mem_block_t *) (old_ptr - (size_t) sizeof(__myfs_mem_block_t));
    s = old_mem_block->size;
    if (size < s) {
        s = size;
    }

    new_mem_block = offset_to_ptr(handle, new_offset);
    memcpy(new_mem_block, old_ptr, s);
    __myfs_free_impl(handle, offset);

    return new_offset;
}

__myfs_inode_t *__myfs_path_resolve(__myfs_handle_t *handle, const char *path) {
  char *index, *__path, *name, file_name[MYFS_MAXIMUM_NAME_LENGTH];
  __myfs_inode_t *node, *child;
  size_t size, i;
  
  if (handle->root_directory == (__myfs_offset_t) 0) {
    struct timespec ts;
    clock_gettime(CLOCK_REALTIME, &ts);
    handle->root_directory = __myfs_allocate_memory(handle, ((size_t) sizeof(__myfs_inode_t)));
    __myfs_inode_t *root = (__myfs_inode_t *) offset_to_ptr(handle, handle->root_directory);
    
    root->name[0] = '/';
    root->name[1] = '\0';
    root->type = DIRECTORY;
    root->modified_time = ts;
    root->accessed_time = ts;
    root->value.directory.number_children = (size_t) 0;
    root->value.directory.children = (__myfs_offset_t) 0;
  }
  
  node = (__myfs_inode_t *) offset_to_ptr(handle, handle->root_directory);
  if (strcmp("/\0", path) == 0) { 
    return node;
  }
    
  __path = (char *) malloc((strlen(path) + 1) * sizeof(char));
  if (__path == NULL) {
    return NULL;
  }
  strcpy(__path, path);
  name = __path + 1;
  index = strchr(name, '/');
  
  while (strlen(name) != 0) {
    child = NULL; 
    if (index != NULL) {
      size = (size_t) (((void *) index) - ((void *) name));
    } else {
      size = (size_t) strlen(name);
    }
    strncpy(file_name, name, size);
    file_name[size] = '\0';
    for (i = 0; i < node->value.directory.number_children; i++) {
      child = (__myfs_inode_t *) offset_to_ptr(handle,(node->value.directory.children + i * ((size_t) sizeof(__myfs_inode_t))));
      if (strcmp(child->name, file_name) == 0) {
	node = child;
	break;
      }
    }
    
    memset(file_name, 0, size);
    
    if (node != child) { 
      free(__path);
      return NULL;
    }
    if (index == NULL) {
      break;
    }
    name = index + 1;
    index = strchr(name, '/');
  }
  free(__path);
  return node;
}



size_t __myfs_total_size(__myfs_handle_t *handle) {
  __myfs_mem_block_t *mem_block;
  size_t max_free_size;
  
  max_free_size = (size_t) 0;
  for (mem_block = (__myfs_mem_block_t *) offset_to_ptr(handle, handle->free_memory);
       mem_block != NULL; 
       mem_block = (__myfs_mem_block_t *) (offset_to_ptr(handle,mem_block->next))) {
        if (mem_block->size > max_free_size) {
          max_free_size = mem_block->size;
        }
  }
  return max_free_size;
}
/* End of helper functions */







/* Implements an emulation of the stat system call on the filesystem 
   of size fssize pointed to by fsptr. 
   
   If path can be followed and describes a file or directory 
   that exists and is accessable, the access information is 
   put into stbuf.  On success, 0 is returned. On failure, -1 is returned and the appropriate error code is put into *errnoptr.

   man 2 stat documents all possible error codes and gives more detail
   on what fields of stbuf need to be filled in. Essentially, only the
   following fields need to be supported:

   st_uid      the value passed in argument
   st_gid      the value passed in argument
   st_mode     (as fixed values S_IFDIR | 0755 for directories,
                                S_IFREG | 0755 for files)
   st_nlink    (as many as there are subdirectories (not files) for directories
                (including . and ..),
                1 for files)
   st_size     (supported only for files, where it is the real file size)
   st_atim
   st_mtim

*/
int __myfs_getattr_implem(void *fsptr, size_t fssize, int *errnoptr,
                          uid_t uid, gid_t gid,
                          const char *path, struct stat *stbuf) {
    __myfs_handle_t *handle; 
    __myfs_inode_t *node;
    char *file_name;
    int counter;
    size_t i;

    handle = __myfs_get_handle(fsptr, fssize);

    if (handle == NULL){
        *errnoptr = EFAULT;  
        return -1;
    }

    node = __myfs_path_resolve(handle, path);

    if (node == NULL) {
        *errnoptr = ENOENT;
        return -1;
    }
    
    file_name = strrchr(path, '/') + 1;
    if (strlen(file_name) >= MYFS_MAXIMUM_NAME_LENGTH) {
        *errnoptr = ENAMETOOLONG;
        return -1;
    }

    memset(stbuf, 0, sizeof(struct stat));

    stbuf->st_uid = uid;
    stbuf->st_gid = gid;
    stbuf->st_atim = node->accessed_time;
    stbuf->st_mtim = node->modified_time;

    if (node->type == DIRECTORY) {
    stbuf->st_mode = S_IFDIR | 0755;
     __myfs_inode_t *children = (__myfs_inode_t *) __off_to_ptr(handle, node->value.directory.children);
        counter = 0;
        for (i = (size_t) 0; i < node->value.directory.number_children; i++) {
            if(children[i].type == DIRECTORY) {
                counter++;
            }
        }
        stbuf->st_nlink = counter;
  } else if (node->type == REG_FILE) {
        stbuf->st_mode = S_IFREG | 0755;
        stbuf->st_size = node->value.file.size;
        stbuf->st_nlink = 1;
  }
  return 0;
}

/* Implements an emulation of the readdir system call on the filesystem 
   of size fssize pointed to by fsptr. 

   If path can be followed and describes a directory that exists and
   is accessable, the names of the subdirectories and files 
   contained in that directory are output into *namesptr. The . and ..
   directories must not be included in that listing.

   If it needs to output file and subdirectory names, the function
   starts by allocating (with calloc) an array of pointers to
   characters of the right size (n entries for n names). Sets
   *namesptr to that pointer. It then goes over all entries
   in that array and allocates, for each of them an array of
   characters of the right size (to hold the i-th name, together 
   with the appropriate '\0' terminator). It puts the pointer
   into that i-th array entry and fills the allocated array
   of characters with the appropriate name. The calling function
   will call free on each of the entries of *namesptr and 
   on *namesptr.

   The function returns the number of names that have been 
   put into namesptr. 

   If no name needs to be reported because the directory does
   not contain any file or subdirectory besides . and .., 0 is 
   returned and no allocation takes place.

   On failure, -1 is returned and the *errnoptr is set to 
   the appropriate error code. 

   The error codes are documented in man 2 readdir.

   In the case memory allocation with malloc/calloc fails, failure is
   indicated by returning -1 and setting *errnoptr to EINVAL.

*/
int __myfs_readdir_implem(void *fsptr, size_t fssize, int *errnoptr,
                          const char *path, char ***namesptr) {

    __myfs_handle_t *handle;
    __myfs_inode_t *node, *child;
    char **names;
    size_t size;

    handle = __myfs_get_handle(fsptr, fssize);
    if (handle == NULL){
        *errnoptr = EFAULT;  
        return -1;
    }

    //printf("READDIR %s\n", path);
    node = __myfs_path_resolve(handle, path);
    if (node == NULL){
        *errnoptr = ENOENT;
        return -1;
    }

    size = node->value.directory.number_children;
    if (size == (size_t) 0) {
        return 0;
    }

    names = (char **) calloc(size, sizeof(char *));
    if (names == NULL){
        *errnoptr = ENOMEM;
        return -1;
    }
    
    for (size_t i = 0; i < size; i++) {
        child = ((__myfs_inode_t *) offset_to_ptr(handle,(node->value.directory.children + i * ((size_t) sizeof(__myfs_inode_t)))));
        names[i] = (char *) calloc(strlen(child->name), sizeof(char));
        strcpy(names[i], child->name);
    }
    *namesptr = names;
    return size;
}



/* Implements an emulation of the mknod system call for regular files
   on the filesystem of size fssize pointed to by fsptr.

   This function is called only for the creation of regular files.

   If a file gets created, it is of size zero and has default
   ownership and mode bits.

   The call creates the file indicated by path.

   On success, 0 is returned.

   On failure, -1 is returned and *errnoptr is set appropriately.

   The error codes are documented in man 2 mknod.

*/
int __myfs_mknod_implem(void *fsptr, size_t fssize, int *errnoptr,
                        const char *path) {
    __myfs_handle_t *handle;
    __myfs_inode_t *node, *child;
    char *file_name, *directory_path;
    size_t directory_length, num_children;
    struct timespec ts;
    clock_gettime(CLOCK_REALTIME, &ts);

    handle = __myfs_get_handle(fsptr, fssize);
    if (handle == NULL){
        *errnoptr = EFAULT;  
        return -1;
    }


    if (((size_t) sizeof(__myfs_inode_t)) > __myfs_total_size(handle)) {
        *errnoptr = ENOMEM;
        return -1;
    }

    file_name = strrchr(path, '/') + 1;
    directory_length = strlen(path) - strlen(file_name);
    if (strlen(file_name) >= MYFS_MAXIMUM_NAME_LENGTH) {
        *errnoptr = ENAMETOOLONG;
        return -1;
    }
    
    directory_path = (char *) malloc((directory_length+1) * sizeof(char));
    if (directory_path == NULL) {
        *errnoptr = ENOMEM;
        return -1;
    }

    strncpy(directory_path, path, directory_length);
    directory_path[directory_length] = '\0';
    
    node = __myfs_path_resolve(handle, directory_path);
    if (node == NULL) {
        free(directory_path);
        *errnoptr = ENOENT;
        return -1;
    }

    node->value.directory.number_children++;
    num_children = node->value.directory.number_children;

    if (num_children == 1) {
       node->value.directory.children = __myfs_allocate_memory(handle, ((size_t) sizeof(__myfs_inode_t)));
        if (node->value.directory.children == (__myfs_offset_t) 0) {
            *errnoptr = ENOMEM;
            return -1;
        }
    } else {
       node->value.directory.children = __myfs_reallocate_memory(handle,
                node->value.directory.children, num_children * ((size_t) sizeof(__myfs_inode_t)));
        if (node->value.directory.children == (__myfs_offset_t) 0) {
            *errnoptr = ENOMEM;
            return -1;
        }
    }

    child = (__myfs_inode_t *) offset_to_ptr(handle, (node->value.directory.children + (num_children - 1) * ((size_t) sizeof(__myfs_inode_t)))); 

    strcpy(child->name, file_name);
    child->type = REG_FILE;
    child->modified_time = ts;
    child->accessed_time = ts;
    child->value.file.size = (size_t) 0;
    child->value.file.first_block = (__myfs_offset_t) 0;

    free(directory_path);
    return 0;
}

/* Implements an emulation of the unlink system call for regular files
   on the filesystem of size fssize pointed to by fsptr.

   This function is called only for the deletion of regular files.

   On success, 0 is returned.

   On failure, -1 is returned and *errnoptr is set appropriately.

   The error codes are documented in man 2 unlink.

*/
int __myfs_unlink_implem(void *fsptr, size_t fssize, int *errnoptr,
                        const char *path) {

  __myfs_handle_t *handle;
  __myfs_inode_t *file_node, *dir_node, *node;
  char *file_name, *dir_path;
  size_t dir_len;
  __myfs_file_block_t *prev, *file_block;
  
  handle = __myfs_get_handle(fsptr, fssize);
  if (handle == NULL) {
    *errnoptr = EFAULT;  
    return -1;
  }
  
  
  file_node = __myfs_path_resolve(handle, path);
  if (file_node == NULL) {
    *errnoptr = ENOENT;
    return -1;
  }
  
  if (file_node->type == DIRECTORY) {
    *errnoptr = EISDIR;
    return -1;
  }
  
  file_name = strrchr(path, '/') + 1;
  dir_len = strlen(path) - strlen(file_name);
  
  dir_path = (char *) malloc((dir_len+1) * sizeof(char));
  if (dir_path == NULL) {
    *errnoptr = ENOMEM;
    return -1;
  }
  strncpy(dir_path, path, dir_len);
  dir_path[dir_len] = '\0';
  
  dir_node = __myfs_path_resolve(handle, dir_path);
  if (dir_node == NULL) {
    free(dir_path);
    *errnoptr = ENOENT;
    return -1;
  }
  
  for (size_t i = 0; i < dir_node->value.directory.number_children; i++) {
    node = (__myfs_inode_t *) offset_to_ptr(handle,dir_node->value.directory.children + i * ((size_t) sizeof(__myfs_inode_t)));
    if (strcmp(node->name, file_name) == 0) {
      break;
    }
  }
  
  for (file_block = (__myfs_file_block_t *) offset_to_ptr(handle,node->value.file.first_block),
       prev = NULL; file_block != NULL;
       prev = file_block, file_block = (__myfs_file_block_t *) offset_to_ptr(handle,file_block->next)) {
    
    if (file_block->data != (__myfs_offset_t) 0) file_block->data = __myfs_reallocate_memory(handle,file_block->data, (size_t) 0);
    if (prev != NULL) {
      prev = (__myfs_file_block_t *) __myfs_reallocate_memory(handle, ptr_to_offset((void *) prev, handle), (size_t) 0);
    } 
  }
  
  if (prev != NULL) {
    prev = (__myfs_file_block_t *) __myfs_reallocate_memory(handle, ptr_to_offset((void *) prev, handle), (size_t) 0);
  }

  node->value.file.size = (size_t) 0;
  
  if (dir_node->value.directory.number_children > 1) {
    memcpy((void *) node, offset_to_ptr(handle, (dir_node->value.directory.children + (dir_node->value.directory.number_children - 1) * ((size_t) sizeof(__myfs_inode_t)))), ((size_t) sizeof(__myfs_inode_t)));
  }

  dir_node->value.directory.number_children--;
  dir_node->value.directory.children = __myfs_reallocate_memory(handle,dir_node->value.directory.children, (dir_node->value.directory.number_children * ((size_t) sizeof(__myfs_inode_t))));
  
  free(dir_path);
  return 0;
}

/* Implements an emulation of the rmdir system call on the filesystem 
   of size fssize pointed to by fsptr. 

   The call deletes the directory indicated by path.

   On success, 0 is returned.

   On failure, -1 is returned and *errnoptr is set appropriately.

   The function call must fail when the directory indicated by path is
   not empty (if there are files or subdirectories other than . and ..).

   The error codes are documented in man 2 rmdir.

*/
int __myfs_rmdir_implem(void *fsptr, size_t fssize, int *errnoptr,
                        const char *path) {

    __myfs_handle_t *handle;
    __myfs_inode_t *file_node, *dir_node, *node;
    char *dir_name, *dir_path;
    size_t dir_len;

    handle = __myfs_get_handle(fsptr, fssize);
    if (handle == NULL){
        *errnoptr = EFAULT;  
        return -1;
    }

   // printf("RMDIR %s\n", path);

    file_node = __myfs_path_resolve(handle, path);
    if (file_node == NULL){
        *errnoptr = ENOENT;
        return -1;
    }

    if (file_node->value.directory.number_children != 0){
        *errnoptr = ENOTEMPTY;
        return -1;
    }

    dir_name = strrchr(path, '/') + 1;
    dir_len = strlen(path) - strlen(dir_name);

    dir_path = (char *) malloc((dir_len+1) * sizeof(char));
    if (dir_path == NULL){
        *errnoptr = ENOMEM;
        return -1;
    }
    strncpy(dir_path, path, dir_len);
    dir_path[dir_len] = '\0';
    
    dir_node = __myfs_path_resolve(handle, dir_path);
    if (dir_node == NULL){
        free(dir_path);
        *errnoptr = ENOENT;
        return -1;
    }


    for (size_t i = 0; i < dir_node->value.directory.number_children; i++){
        node = (__myfs_inode_t *) offset_to_ptr(handle,
               dir_node->value.directory.children + i * ((size_t) sizeof(__myfs_inode_t)));

        if (strcmp(node->name, dir_name) == 0){
            break;
        }
    }

    if (dir_node->value.directory.number_children > 1) {
        memcpy((void *) node, offset_to_ptr(handle, (dir_node->value.directory.children 
                + (dir_node->value.directory.number_children - 1) * ((size_t) sizeof(__myfs_inode_t)))),
            ((size_t) sizeof(__myfs_inode_t)));
    }

    dir_node->value.directory.number_children--;
    dir_node->value.directory.children = __myfs_reallocate_memory(handle,
            dir_node->value.directory.children, (dir_node->value.directory.number_children
                * ((size_t) sizeof(__myfs_inode_t))));

    free(dir_path);
    return 0;
}

/* Implements an emulation of the mkdir system call on the filesystem 
   of size fssize pointed to by fsptr. 

   The call creates the directory indicated by path.

   On success, 0 is returned.

   On failure, -1 is returned and *errnoptr is set appropriately.

   The error codes are documented in man 2 mkdir.

*/
int __myfs_mkdir_implem(void *fsptr, size_t fssize, int *errnoptr,
                        const char *path) {
    __myfs_handle_t *handle;
    __myfs_inode_t *node, *child, *dir_node;
    char *dir_name, *dir_path;
    size_t dir_len, num_children;
    struct timespec ts;
    clock_gettime(CLOCK_REALTIME, &ts);

    handle = __myfs_get_handle(fsptr, fssize);
    if (handle == NULL){
        *errnoptr = EFAULT;  
        return -1;
    }

    if (((size_t) sizeof(__myfs_inode_t)) > __myfs_total_size(handle)){
        *errnoptr = ENOMEM;
        return -1;
    }

    dir_node = __myfs_path_resolve(fsptr, path);
    if (dir_node != NULL){
        *errnoptr = EEXIST;
        return -1;
    }

    dir_name = strrchr(path, '/') + 1;
    dir_len = strlen(path) - strlen(dir_name);
    if (strlen(dir_name) >= MYFS_MAXIMUM_NAME_LENGTH){
        *errnoptr = ENAMETOOLONG;
        return -1;
    }

    dir_path = malloc((dir_len+1) * sizeof(char));
    if (dir_path == NULL){
        *errnoptr = ENOMEM;
        return -1;
    }
    strncpy(dir_path, path, dir_len);
    dir_path[dir_len] = '\0';
    
    node = __myfs_path_resolve(handle, dir_path);
    if (node == NULL) {
        free(dir_path);
        *errnoptr = ENOENT;
        return -1;
    }

    node->value.directory.number_children++;
    num_children = node->value.directory.number_children;

    if (num_children == 1) {
       node->value.directory.children = __myfs_allocate_memory(handle, ((size_t) sizeof(__myfs_inode_t)));
        if (node->value.directory.children == (__myfs_offset_t) 0) {
            *errnoptr = ENOMEM;
            return -1;
        }
    } else {
       node->value.directory.children = __myfs_reallocate_memory(handle, node->value.directory.children, num_children * ((size_t) sizeof(__myfs_inode_t)));
        if (node->value.directory.children == (__myfs_offset_t) 0) {
            *errnoptr = ENOMEM;
            return -1;
        }
    }

    child = (__myfs_inode_t *) offset_to_ptr(handle, (node->value.directory.children  + (num_children-1) * ((size_t) sizeof(__myfs_inode_t)))); 

    strcpy(child->name, dir_name);
    child->type = DIRECTORY;
    child->modified_time = ts;
    child->accessed_time = ts;
    child->value.directory.number_children = (size_t) 0;
    child->value.directory.children = (__myfs_offset_t) 0;

    free(dir_path);
    return 0;
}

/* Implements an emulation of the rename system call on the filesystem 
   of size fssize pointed to by fsptr. 

   The call moves the file or directory indicated by from to to.

   On success, 0 is returned.

   On failure, -1 is returned and *errnoptr is set appropriately.

   Caution: the function does more than what is hinted to by its name.
   In cases the from and to paths differ, the file is moved out of 
   the from path and added to the to path.

   The error codes are documented in man 2 rename.

*/
int __myfs_rename_implem(void *fsptr, size_t fssize, int *errnoptr,
                         const char *from, const char *to) {

  __myfs_handle_t *handle;
  __myfs_inode_t *from_file, *from_dir, *to_dir;
  char *from_file_name, *to_file_name, *from_dir_name, *to_dir_name;
  size_t from_dir_len, to_dir_len;
  
  if (strcmp(from , to) == 0)
    return 0;
  
  handle = __myfs_get_handle(fsptr, fssize);
  if (handle == NULL) {
    *errnoptr = EFAULT;  
    return -1;
  }
  
  from_file = __myfs_path_resolve(handle, from);
  if (from_file == NULL) {
    *errnoptr = ENOENT;
    return -1;
  }
  
  to_file_name = strrchr(to, '/') + 1;
  to_dir_len = strlen(to) - strlen(to_file_name);
  from_file_name = strrchr(from, '/') + 1;
  from_dir_len = strlen(from) - strlen(from_file_name);
  
  if (strlen(to_file_name) >= MYFS_MAXIMUM_NAME_LENGTH) {
    *errnoptr = ENAMETOOLONG;
    return -1;
  }
  
  to_dir_name = (char *) malloc((to_dir_len + 1) * sizeof(char));
  from_dir_name = (char *) malloc((from_dir_len+1) * sizeof(char));
  if (from_dir_name == NULL || to_dir_name == NULL) {
    *errnoptr = ENOMEM;
    return -1;
  }
  
  strncpy(from_dir_name, from, from_dir_len);
  from_dir_name[from_dir_len] = '\0';
  
  strncpy(to_dir_name, to, to_dir_len);
  to_dir_name[to_dir_len] = '\0';
  
  from_dir = __myfs_path_resolve(handle, from_dir_name);
  if (from_dir == NULL){
    free(from_dir_name);
    free(to_dir_name);
    *errnoptr = ENOENT;
    return -1;
  }
  
  to_dir = __myfs_path_resolve(handle, to_dir_name);
  if (to_dir == NULL) {
    free(from_dir_name);
    free(to_dir_name);
    *errnoptr = ENOENT;
    return -1;
  }
  
  strcpy(from_file->name, to_file_name);
  
  if (strcmp(from_dir_name, to_dir_name) == 0) {
    free(from_dir_name);
    free(to_dir_name);
    return 0;
  }
  
  to_dir->value.directory.number_children++;
  if (to_dir->value.directory.children == (__myfs_offset_t) 0) {
    to_dir->value.directory.children = __myfs_allocate_memory(handle,(to_dir->value.directory.number_children * ((size_t) sizeof(__myfs_inode_t))));
  } else {
    to_dir->value.directory.children = __myfs_reallocate_memory(handle,to_dir->value.directory.children, (to_dir->value.directory.number_children * ((size_t) sizeof(__myfs_inode_t))));
  }
  
  memmove(offset_to_ptr(handle, (to_dir->value.directory.children + (to_dir->value.directory.number_children - 1) * ((size_t) sizeof(__myfs_inode_t)))), (void *) from_file, ((size_t) sizeof(__myfs_inode_t)));
  
  from_file->value.file.size = (size_t) 0;
  if (from_dir->value.directory.number_children > 1) {
    memmove((void *) from_file, offset_to_ptr(handle,(from_dir->value.directory.children + (from_dir->value.directory.number_children - 1) * ((size_t) sizeof(__myfs_inode_t)))), ((size_t) sizeof(__myfs_inode_t)));
  }
  from_dir->value.directory.number_children--;
  from_dir->value.directory.children = __myfs_reallocate_memory(handle,from_dir->value.directory.children, (from_dir->value.directory.number_children * ((size_t) sizeof(__myfs_inode_t))));
  
  free(from_dir_name);
  free(to_dir_name);
  return 0;
}

/* Implements an emulation of the truncate system call on the filesystem 
   of size fssize pointed to by fsptr. 
   
   The call changes the size of the file indicated by path to offset
   bytes.

   When the file becomes smaller due to the call, the extending bytes are
   removed. When it becomes larger, zeros are appended.

   On success, 0 is returned.

   On failure, -1 is returned and *errnoptr is set appropriately.

   The error codes are documented in man 2 truncate.
   
*/
int __myfs_truncate_implem(void *fsptr, size_t fssize, int *errnoptr,
                           const char *path, off_t offset) {
  
  __myfs_handle_t *handle; 
  __myfs_inode_t *node;
  __myfs_file_block_t *file_block, *prev;
  off_t new_offset;
  
  handle = __myfs_get_handle(fsptr, fssize);
  if (handle == NULL){
    *errnoptr = EFAULT;  
    return -1;
  }
  
  node = __myfs_path_resolve(handle, path);
  if (node == NULL){
    *errnoptr = ENOENT;
    return -1;
  }
  
  if (node->type == DIRECTORY) { 
    *errnoptr = EISDIR;
    return -1;
  }
  
  if (node->value.file.size == (size_t) 0) {
    if ((offset + ((size_t) sizeof(__myfs_file_block_t))) > (off_t) __myfs_total_size(handle)) {
      *errnoptr = ENOMEM;
      return -1;
    }
    
    node->value.file.first_block = __myfs_allocate_memory(handle, ((size_t) sizeof(__myfs_file_block_t)));
    
    if (node->value.file.first_block == (__myfs_offset_t) 0) {
      *errnoptr = ENOMEM;
      return -1;
    }
    
    file_block = (__myfs_file_block_t *) offset_to_ptr(handle,node->value.file.first_block);
    node->value.file.size = offset;
    
  } else if (offset > node->value.file.size) {
    if ((offset + ((size_t) sizeof(__myfs_file_block_t))) > (off_t) __myfs_total_size(handle)) {
      *errnoptr = ENOMEM;
      return -1;
    }
    for (file_block = (__myfs_file_block_t *) offset_to_ptr(handle,node->value.file.first_block), prev = NULL; 
	 file_block != NULL;
	 prev = file_block, file_block = (__myfs_file_block_t *) offset_to_ptr(handle,file_block->next));
    
    prev->next = __myfs_allocate_memory(handle, ((size_t) sizeof(__myfs_file_block_t)));
    if (prev->next == (__myfs_offset_t) 0) {
      *errnoptr = ENOMEM;
      return -1;
    }
    
    file_block = (__myfs_file_block_t *) offset_to_ptr(handle, prev->next);
    node->value.file.size = offset;
  } else {
    node->value.file.size = offset;
    new_offset = offset; 
    for (file_block = (__myfs_file_block_t *) offset_to_ptr(handle,node->value.file.first_block); file_block != NULL;
	  file_block = (__myfs_file_block_t *) offset_to_ptr(handle,file_block->next)) {
      
      if (new_offset > file_block->size) {
        new_offset -= file_block->size;
      } else {
	      break;
      }
    }
    
    file_block->data = __myfs_reallocate_memory(handle, file_block->data, new_offset);
    file_block->size = new_offset;
    prev = NULL;
    file_block = (__myfs_file_block_t *) offset_to_ptr(handle, file_block->next);
    while (file_block != NULL) {
      if (prev != NULL) {
	      prev = (__myfs_file_block_t *) __myfs_reallocate_memory(handle, ptr_to_offset((void *)prev, handle), (size_t) 0);
      }
      
      __myfs_free_impl(handle, file_block->data);
      prev = file_block;
      file_block = (__myfs_file_block_t *) offset_to_ptr(handle,file_block->next);
    }
    if (prev != NULL) {
      __myfs_free_impl(handle, ptr_to_offset((void *) prev, handle));
    }
    return 0;
  }
    
  file_block->next = (__myfs_offset_t) 0;
  file_block->data = __myfs_allocate_memory(handle, offset);
  if (file_block->data == (__myfs_offset_t) 0) {
    *errnoptr = ENOMEM;
    return -1;
  }
  file_block->size = offset;
  memset(offset_to_ptr(handle, file_block->data), '\0', offset);
  
  return 0;
}

/* Implements an emulation of the open system call on the filesystem 
   of size fssize pointed to by fsptr, without actually performing the opening
   of the file (no file descriptor is returned).

   The call just checks if the file (or directory) indicated by path
   can be accessed, i.e. if the path can be followed to an existing
   object for which the access rights are granted.

   On success, 0 is returned.

   On failure, -1 is returned and *errnoptr is set appropriately.

   The two only interesting error codes are 

   * EFAULT: the filesystem is in a bad state, we can't do anything

   * ENOENT: the file that we are supposed to open doesn't exist (or a
             subpath).

   It is possible to restrict ourselves to only these two error
   conditions. It is also possible to implement more detailed error
   condition answers.

   The error codes are documented in man 2 open.

*/
int __myfs_open_implem(void *fsptr, size_t fssize, int *errnoptr,
                       const char *path) {
  __myfs_handle_t *handle; 
  __myfs_inode_t *node;
  
  
  handle = __myfs_get_handle(fsptr, fssize);
  if (handle == NULL) {
    *errnoptr = EFAULT;  
    return -1;
  }
  
  node = __myfs_path_resolve(handle, path);
  if (node == NULL) {
    *errnoptr = ENOENT;
    return -1;
  }
  
  return 0;
}

/* Implements an emulation of the read system call on the filesystem 
   of size fssize pointed to by fsptr.

   The call copies up to size bytes from the file indicated by 
   path into the buffer, starting to read at offset. See the man page
   for read for the details when offset is beyond the end of the file etc.
   
   On success, the appropriate number of bytes read into the buffer is
   returned. The value zero is returned on an end-of-file condition.

   On failure, -1 is returned and *errnoptr is set appropriately.

   The error codes are documented in man 2 read.

*/
int __myfs_read_implem(void *fsptr, size_t fssize, int *errnoptr,
                       const char *path, char *buf, size_t size, off_t offset) {
  __myfs_handle_t *handle; 
  __myfs_inode_t *node;
  __myfs_file_block_t *file_block;
  size_t new_size;
  off_t new_offset;
  int num_bytes = 0;
  
  handle = __myfs_get_handle(fsptr, fssize);
  if (handle == NULL){
    *errnoptr = EFAULT;  
    return -1;
  }
  
  node = __myfs_path_resolve(handle, path);
  if (node == NULL) {
    *errnoptr = ENOENT;
    return -1;
  }

  if (node->type == DIRECTORY) {
    *errnoptr = EISDIR;
    return -1;
  }
  
  if ((size_t) offset > node->value.file.size) {
    return 0;
  }
  
  new_size = size;
  new_offset = offset;
  for (file_block = (__myfs_file_block_t *) offset_to_ptr(handle,node->value.file.first_block);
       file_block != NULL;
       file_block = (__myfs_file_block_t *) offset_to_ptr(handle,file_block->next)) {
    if (new_offset > file_block->size) {
      new_offset -= file_block->size;
    } else {
      break;
    }
  }
  while (file_block != NULL) {
    if (((size_t) new_offset + new_size) < file_block->size) {
      memcpy(buf+num_bytes, offset_to_ptr(handle, ((size_t) new_offset + file_block->data)), new_size);
      num_bytes += (int) new_size;
      break;
    } else {
      new_size -= (file_block->size - new_offset);     
      memcpy(buf + num_bytes, offset_to_ptr(handle, ((size_t) new_offset + file_block->data)), (file_block->size - new_offset));
      num_bytes += (int) (file_block->size - new_offset);
    }
    
    if (new_offset != (off_t) 0) {
      new_offset = 0;
    }
    
    file_block = (__myfs_file_block_t *) offset_to_ptr(handle, file_block->next);
  }
  return num_bytes;
}

/* Implements an emulation of the write system call on the filesystem 
   of size fssize pointed to by fsptr.
   
   The call copies up to size bytes to the file indicated by 
   path into the buffer, starting to write at offset. See the man page
   for write for the details when offset is beyond the end of the file etc.
   
   On success, the appropriate number of bytes written into the file is
   returned. The value zero is returned on an end-of-file condition.

   On failure, -1 is returned and *errnoptr is set appropriately.

   The error codes are documented in man 2 write.

*/
int __myfs_write_implem(void *fsptr, size_t fssize, int *errnoptr,
                        const char *path, const char *buf, size_t size, off_t offset) {
  __myfs_handle_t *handle; 
  __myfs_inode_t *node;
  __myfs_file_block_t *file_block;
  int num_bytes;
  
  handle = __myfs_get_handle(fsptr, fssize);
  if (handle == NULL) {
    *errnoptr = EFAULT;  
    return -1;
  }
  
  node = __myfs_path_resolve(handle, path);
  if (node == NULL) {
    *errnoptr = ENOENT;
    return -1;
  }
  
  if (node->type == DIRECTORY) {
    *errnoptr = EISDIR;
    return -1;
  }
  
  if (node->value.file.size == (size_t) 0) {
    node->value.file.first_block = __myfs_allocate_memory(handle, ((size_t) sizeof(__myfs_file_block_t)));
    
    if (node->value.file.first_block == (size_t) 0) {
      *errnoptr = ENOMEM;
      return -1;
    }
  }
  
  if (offset > node->value.file.size) {
    return 0;
  }
  
  file_block = (__myfs_file_block_t *) offset_to_ptr(handle, node->value.file.first_block);
  
  if (node->value.file.size == (size_t) 0) {
    file_block->data = __myfs_allocate_memory(handle, size);
    
    if (file_block->data == (size_t) 0) {
      *errnoptr = ENOMEM;
      return -1;
    }
    
    file_block->size = size;
    file_block->next = (__myfs_offset_t) 0;
    node->value.file.size = size;
  }
    
  else if (((size_t) offset + size) > node->value.file.size) {
    for (file_block = (__myfs_file_block_t*) offset_to_ptr(handle,node->value.file.first_block);
	 file_block->next != (__myfs_offset_t) 0;
	 file_block = (__myfs_file_block_t *) offset_to_ptr(handle,file_block->next));
    
    file_block->next = __myfs_allocate_memory(handle, ((size_t) sizeof(__myfs_file_block_t)));
    
    if (file_block->next == (size_t) 0){
      *errnoptr = ENOMEM;
      return -1;
    }
    
    file_block = (__myfs_file_block_t *) offset_to_ptr(handle,file_block->next);
    file_block->data = __myfs_allocate_memory(handle, size);
    
    if (file_block->data == (size_t) 0) {
      *errnoptr = ENOMEM;
      return -1;
    }
    file_block->next = (__myfs_offset_t) 0;
    file_block->size = size;
    node->value.file.size += size;
  }
  num_bytes = (size_t) size;
  memcpy(offset_to_ptr(handle, file_block->data), buf, size);
  
  return num_bytes;
}

/* Implements an emulation of the utimensat system call on the filesystem 
   of size fssize pointed to by fsptr.

   The call changes the access and modification times of the file
   or directory indicated by path to the values in ts.

   On success, 0 is returned.

   On failure, -1 is returned and *errnoptr is set appropriately.

   The error codes are documented in man 2 utimensat.
   
*/
int __myfs_utimens_implem(void *fsptr, size_t fssize, int *errnoptr,
                          const char *path, const struct timespec ts[2]) {
  
  __myfs_handle_t *handle; 
  __myfs_inode_t *node;
  
  handle = __myfs_get_handle(fsptr, fssize);
  
  if (handle == NULL) {
    *errnoptr = EFAULT;  
    return -1;
  }
  
  node = __myfs_path_resolve(handle, path);
  
  if (node == NULL) {
    *errnoptr = ENOENT;
      return -1;
  }
  
  node->accessed_time = ts[0];
  node->modified_time = ts[1];
  
  return 0;
}

/* Implements an emulation of the statfs system call on the filesystem 
   of size fssize pointed to by fsptr.
   
   The call gets information of the filesystem usage and puts in 
   into stbuf.
   
   On success, 0 is returned.
   
   On failure, -1 is returned and *errnoptr is set appropriately.
   
   The error codes are documented in man 2 statfs.
   
   Essentially, only the following fields of struct statvfs need to be
   supported:
   
   f_bsize   fill with what you call a block (typically 1024 bytes)
   f_blocks  fill with the total number of blocks in the filesystem
   f_bfree   fill with the free number of blocks in the filesystem
   f_bavail  fill with same value as f_bfree
   f_namemax fill with your maximum file/directory name, if your
   filesystem has such a maximum
	     
*/
int __myfs_statfs_implem(void *fsptr, size_t fssize, int *errnoptr,
                         struct statvfs* stbuf) {
  
  __myfs_handle_t *handle;
  handle = __myfs_get_handle(fsptr,fssize);
  
  // size_t size = MYFS_size ((size_t) 1024); 
  
  if (handle == NULL) {
    *errnoptr = EFAULT;
    return -1;
  }
  
  /* How many blocks are our size? - 1024 | (1 byte, can use other defined sizes though)
     Iterates over free space offsets and computes the total amount of free memory left  */
  memset(stbuf, 0, sizeof(statvfs));
  
  /* Block size is standard size of 1024 | blocks = size / SIZE */ 
  stbuf->f_bsize = MYFS_size; 
  stbuf->f_blocks = ((fsblkcnt_t) (handle->size / MYFS_size));
  stbuf->f_bfree = ((fsblkcnt_t) (free_space(handle) / MYFS_size));
  stbuf->f_bavail = stbuf->f_bfree;
  stbuf->f_namemax = (u_long) MYFS_MAXIMUM_NAME_LENGTH; // 256 characters 
  return 0;
}
