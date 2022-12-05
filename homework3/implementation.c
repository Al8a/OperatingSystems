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
#include "implementation.h"


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

static inline void * offset_to_ptr(void *fsptr, __myfs_offset_t off) {
  if (off == ((__myfs_offset_t) 0)) return NULL;
  void *ptr = fsptr + off;
  /* overflow check */
  if (ptr < fsptr) {
    return NULL;
  }
  return ptr;
}


static inline __myfs_offset_t ptr_to_offset(void *fsptr, void *ptr) {
  if (ptr == NULL) return ((__myfs_offset_t)0);
  if (ptr <= fsptr) return ((__myfs_offset_t)0);
  return (__myfs_offset_t)(ptr - fsptr);
}



//static __myfs_inode_t *__myfs_path_resolve(__myfs_handle_t handle, const char *path);
//static __myfs_inode_t * __myfs_path_resolve_one_step(__myfs_handle_t handle, __myfs_inode_t *curr, int *errnoptr); //incomplete

/*
  static void __myfs_set_filename(char *dst, const char *src) {
  dst[MYFS_MAXIMUM_NAME_LENGTH -1] = '\0';
  strncpy(dest, src, MYFS_MAXIMUM_NAME_LENGTH - 1);
  }
*/





static void __myfs_set_curr_time(__myfs_inode_t *node, int modified_flag) {
  if (node == NULL) return ;
  struct timespec ts;
  /*  Retrieve time of the specified clock clk_id */
  if (clock_gettime(CLOCK_REALTIME, &ts) == 0) {
    node->times[0] = ts;
    if (modified_flag) {
      node->times[1] = ts;
    }
  }
}



static __myfs_handle_t  __myfs_get_handle(void *fsptr, size_t size) {
  __myfs_handle_t handle;
  __myfs_mem_block_t block;
  size_t s;
  
  if (size < sizeof(struct __myfs_handle_struct_t)) return NULL;
  handle = (__myfs_handle_t) fsptr;

  if (handle->magic != MYFS_MAGIC) {
    s = size - sizeof(struct __myfs_handle_struct_t);
    
    if (handle->magic != (uint32_t) 0) {
      memset(fsptr + sizeof(struct __myfs_handle_struct_t), 0, s);
    }
    handle->magic = MYFS_MAGIC;
    handle->size = s;
    
    if (s == (size_t) 0) {
      handle->free_memory = (__myfs_offset_t) 0;
    }

    else {
      block = (__myfs_mem_block_t) offset_to_ptr(fsptr, sizeof(struct __myfs_handle_struct_t));
      block->size = s;
      block->next = (offset_t) 0;
      handle->free_memory = ptr_to_offset(block, fsptr);
    }
    handle->root_dir = (__myfs_offset_t) 0;
  }
  return handle;
}



__myfs_mem_block_t get_memory_block(__myfs_handle_t handle, size_t size) {
  __myfs_mem_block_t curr, prev, next;
  
  for (curr = (__myfs_mem_block_t *) offset_to_ptr(handle, handle->free_memory),
         prev = NULL; curr != NULL && curr->next != (__myfs_offset_t) 0; prev = curr,
         curr = (__myfs_mem_block_t *) offset_to_ptr(handle, curr->next)) {
    if (curr->size > size) {
      break;
    }
  }
  
  // there does not exist a block with enough size
  if (curr->size < size) {
    return NULL;
  }
  
  if (curr->size - size > (size_t) 0) { // create new next block
    next = (__myfs_mem_block_t *) offset_to_ptr(curr, size);
    next->size = curr->size - size;
    next->next = curr->next;
    
    // already exists a next block
  } else { 
    next = (__myfs_mem_block_t * ) offset_to_ptr(handle, curr->next);
  }
  
  // curr is first available memory block
  if (curr == (__myfs_mem_block_t *) offset_to_ptr(handle, handle->free_memory)) {
    handle->free_memory = ptr_to_offset(next, handle);
  } else {
    prev->next= ptr_to_offset(next, handle);
  }
  curr->size = size; 
  curr->next = (__myfs_offset_t) 0;
  
  return curr;
}


// add block back to free memory linked list
void add_to_free_memory(__myfs_handle_t handle, __myfs_offset_t offset) { 
  __myfs_mem_block_t block, curr, prev;
  block = (__myfs_mem_block_t *) offset_to_ptr(handle, offset);
  
  for (curr = (__myfs_mem_block_t *) offset_to_ptr(handle, handle->free_memory),
    prev = NULL; curr != NULL; prev = curr, 
    curr= (__myfs_mem_block_t * ) offset_to_ptr(handle, curr->next)) {
    
    if ((void *) block < (void *) curr) {
      break;
    }
  }
  
  // place block in between prev and cur block
  if (curr != NULL) {
    block->next = ptr_to_offset(curr, handle);
  }
  else {
    block->next = (__myfs_offset_t) 0;
  }
  
  if (prev == NULL) { // block is new head
    handle->free_memory = offset;      
    
  } else {
    prev->next = offset;
  }
  
  // merge with right block
  if (curr != NULL && ((void *) ((void *) block + block->size)) == ((void *) curr)) {
    block->size += curr->size;
    block->next = curr->next;
  }
  
  // merge with left block
  if (prev != NULL && ((void *) ((void *) prev + prev->size)) == ((void *) block)) {
    prev->size += block->size;
    prev->next= block->next;
  }
  return;
}



size_t free_size(__myfs_handle_t handle) {
  size_t total_free_size;
  __myfs_mem_block_t block;
  
  total_free_size = (size_t) 0;
  for (block = (__myfs_mem_block_t *) offset_to_ptr(handle, handle->free_memory);
       block != NULL; block = (__myfs_mem_block_t) (offset_to_ptr(handle, block->next))) {
    total_free_size += block->size; 
  }
  return total_free_size;
}



size_t max_size(__myfs_handle_t handle) {
  size_t max_free_size;
  __myfs_mem_block_t block;
  
  max_free_size = (size_t) 0;
  for (block = (__myfs_mem_block_t *) offset_to_ptr(handle, handle->free_memory);
       block != NULL; block = (__myfs_mem_block_t *) (offset_to_ptr(handle, block->next))) {
    if (block->size > max_free_size) {
      max_free_size = block->size;
    }
  }
  return max_free_size;
}



//static void __myfs_set_filename(char *dst, const char *src)


/* Returns 1 unless ch is one of the following illegal naming chars:
// '{' , '}', '|', '~', 'DEL', ':', '/', '\', and ','
static int inode_name_charvalid(char ch) {
  return  (!(ch < 32 || ch == 44 || ch == 95 || ch  == 47 || ch == 58 || ch > 122));
}
*/


/* Returns 1 iff name is legal ascii chars and within max length, else 0.
static int inode_name_isvalid(char *name) {
  int len = 0;
  
  for (char *c = name; *c != '\0'; c++) {
    len++;
    if (len > MYFS_MAXIMUM_NAME_LENGTH) return 0; // Check for over max length
    if (!inode_name_charvalid(*c)) return 0;      // Check for illegal chars
  }
  
  if (len == 0) return 0;  // Zero length is invalid
  return 1;                // Valid
}
*/

/* If size is zero, return NULL. Otherwise, call get_allocation with the designated passed size. */
void *__malloc_impl(void *fsptr, void *ptr, size_t *size) {
  if (size == ((size_t) 0)) return NULL;
  if (ptr == NULL) {
    ptr = fsptr + sizeof(size_t);
  }
  return get_allocation(fsptr, get_free_memory_ptr(fsptr), ptr - sizeof(size_t), size);
}


/* If size is less than what already assign to *ptr just lock what is after size and add it using add_allocation_space. */
void *__realloc_impl(void *fsptr, void *ptr, size_t *size) {
  if (*size == ((size_t) 0)) {
    __free_impl(fsptr, ptr);
    return NULL;
  }
  linkedList *LL = get_free_memory_ptr(fsptr);
  if (ptr == fsptr) {
    return get_allocation(fsptr, LL, fsptr, size);
  }
  s_allocation *allocation = (s_allocation *) (((void *) ptr) - sizeof(size_t));
  s_allocation *temp_allocation;
  void *new_ptr = NULL;
  if ((allocation->remaining_memory >= *size) && (allocation->remaining_memory < (*size + sizeof(s_allocation))) ) {
    new_ptr = ptr;
  }
  else if (allocation->remaining_memory > *size) {
    temp_allocation = (s_allocation *) (ptr + *size);
    temp_allocation->remaining_memory = allocation->remaining_memory - *size - sizeof(size_t);
    temp_allocation->next_space = 0; 
    add_allocation_space(fsptr, LL, temp_allocation);
    allocation->remaining_memory = *size;
    new_ptr = ptr;
  }
  else {
    new_ptr = get_allocation(fsptr, LL, fsptr, size);
    if (*size != 0) {
      return NULL;
    }
    memcpy(new_ptr, ptr, allocation->remaining); 
    add_allocation_space(fsptr, LL, allocation);
  }
  return new_ptr;
}


/* Add space back to List using add_allocation_space */
void __free_impl(void *fsptr, void *ptr)
{
  if (ptr == NULL) return;
  add_allocation_space(fsptr, get_free_memory_ptr(fsptr), ptr - sizeof(size_t));
}


/* Make an s_Allocation item using length and start and add it to the list which does it in ascending order */
void add_allocation_space(void *fsptr, List *LL, s_Allocation *alloc)
{
  s_Allocation *temp;
  __myfs_off_t temp_off = LL->first_space;
  __myfs_off_t alloc_off = ptr_to_off(fsptr, alloc);
  
  //New space address is less than the first_space in LL
  if (temp_off > alloc_off) {
    /* At this point we know that alloc comes before LL->first_space */
    LL->first_space = alloc_off; //Update first space
    //Check if we can merge LL->first_space and alloc
    if ( (alloc_off + sizeof(size_t) + alloc->remaining) == temp_off ) {
      //Get first pointer
      temp = off_to_ptr(fsptr, temp_off);
      //Combine the spaces available
      alloc->remaining += sizeof(size_t) + temp->remaining;
      //Update pointers
      alloc->next_space = temp->next_space;
    }
    //We couldn't merge so we just add it as the first_space and update pointers
    else {
      alloc->next_space = temp_off;
    }
  }
  //Find after what pointer does alloc should be added
  else {
    temp = off_to_ptr(fsptr, temp_off);
    //Get the last pointer that is in lower memory than alloc
    while ( (temp->next_space != 0) && (temp->next_space < alloc_off) ) {
      temp = off_to_ptr(fsptr, temp->next_space);
    }
    temp_off = ptr_to_off(fsptr, temp);
    
    //At this point, temp_off < alloc_off < temp->next_space. But, there is no guaranty that temp->next_space != 0 (NULL)
    
    //If temp->next_space != 0 we make alloc_off to point to it and try to merge them
    __myfs_off_t after_alloc_off = temp->next_space;
    if (after_alloc_off != 0) {
      //Check if we can merge alloc and after_alloc
      if ( (alloc_off + sizeof(size_t) + alloc->remaining) == after_alloc_off ) {
        s_Allocation *after_alloc = off_to_ptr(fsptr, after_alloc_off);
        alloc->remaining += sizeof(size_t) + after_alloc->remaining;
        alloc->next_space = after_alloc->next_space;
      }
      //We couldn't merge them
      else {
        alloc->next_space = after_alloc_off;
      }
    }
    //alloc is the last space available in memory ascending order
    else {
      alloc->next_space = 0;
    }
    //Try to merge temp and alloc
    if ( (temp_off + sizeof(size_t) + temp->remaining) == alloc_off ) {
      temp->remaining += sizeof(size_t) + alloc->remaining;
      temp->next_space = alloc->next_space;
    }
    //We couldn't merge them
    else {
      temp->next_space = alloc_off;
    }
  }
}
/* Check if the offset for pref_ptr is 0, if so we get any block for size, otherwise we try to find the block after it and get as much from
 * it as possible and get the rest from the largest block
 */
void *get_allocation(void *fsptr, linkedList *LL, s_allocation *org_pref, size_t *size)
{
  //There is no guarantee that its offset is not 0, if so, we don't consider it
  __myfs_off_t pref_off = ((__myfs_off_t) 0);
  s_Allocation *before_pref = NULL;
  int pref_found = 0;
  
  //Before current space
  __myfs_off_t before_temp_off;
  s_Allocation *before_temp;
  
  //current space
  __myfs_off_t temp_off;
  s_Allocation *temp;
  
  //Largest block variables
  s_Allocation *before_largest = NULL;
  __myfs_off_t largest_off;
  s_Allocation *largest;
  size_t largest_size;
  
  //Use this ptr if a new block needs to be return
  s_Allocation *ptr = NULL;
  
  //Save first space
  before_temp_off = LL->first_space;
  
  //If before_temp have an offset of zero we have use all possible space in memory
  if (!before_temp_off) {
    return NULL;
  }
  
  if (((void *) org_pref) != fsptr) {
    pref_off = ptr_to_off(fsptr, org_pref) + org_pref->remaining_memory;
    //Check that the first block is the prefer one or not
    if (pref_off == before_temp_off) {
      pref_found = 1;
    }
  }
  
  //We currently have before_temp as our largest block
  before_temp = off_to_ptr(fsptr, before_temp_off);
  
  largest_off = before_temp_off;
  largest = before_temp;
  largest_size = before_temp->remaining_memory;
  
  //Get next space
  temp_off = before_temp->next_space;
  
  //Check that size is at least a sizeof(__myfs_off_t)
  if (*size < sizeof(__myfs_offset_t)) {
    *size = sizeof(__myfs_offset_t);
  }
  
  //Iterate the list until the first block that can hold size and the block after pref_ptr is found (or pass because it is not there)
  while (temp_off != ((__myfs_offset_t) 0)) {
    //The offset is to get after the size_t so we readjust it
    temp = off_to_ptr(fsptr, temp_off);
    //Check if temp_off is the prefer block that we are looking for or if temp have more space available than the previous largest
    if ( (pref_off == temp_off) || (temp->remaining_memory > largest_size) ) {
      //If temp_off is pref we would not like to update largest space so we have two places to get space from
      if (pref_off == temp_off) {
        //pref_found was successfully found
        //TODO: extend_pref_block();
        pref_found = 1;
        before_pref = before_temp;
      }
      //Update largest space
      else {
        before_largest = before_temp;
        largest_off = temp_off;
        largest = temp;
        largest_size = temp->remaining_memory;
      }
    }
    //Update pointers to next space
    before_temp_off = temp_off;
    before_temp = temp;
    temp_off = temp->next_space;
  }
  
  //If the prefer block was found we get as much as we can from it until size or until we run out of bytes available from it
  if (pref_found) {
    s_Allocation *pref = off_to_ptr(fsptr, pref_off);
    //Check if you can get all size bytes from the prefer block
    if (pref->remaining_memory >= *size) {
      //Check if we can make an s_Allocation object with the remaining space
      if (pref->remaining_memory > *size + sizeof(s_Allocation)) {
        //Set original pref with final total size
        org_pref->remaining_memory += *size;
        //Make the new s_Allocation object
        temp = ((void *) pref) + *size;
        temp->remaining_memory = pref->remaining_memory - *size;
        temp->next_space = pref->next_space;
        //Update pointers to add temp into list of free blocks
        before_pref->next_space = pref_off + *size;
      }
      //We can't make an s_Allocation object
      else {
        //Add everything that the prefer block have into the original one
        org_pref->remaining_memory += pref->remaining_memory;
        //Update pointers so the one that was pointing to the prefer free block is now pointing to the next free
        before_pref->next_space = pref->next_space;
      }
      *size = ((__myfs_offset_t) 0);
    }
    //We couldn't got everything from the prefer block so we get as much as we can from it
    else {
      //Add everything that the prefer block have into the original one
      org_pref->remaining_memory += pref->remaining_memory;
      //Update pointers so the one that was pointing to the prefer free block is now pointing to the next free
      before_pref->next_space = pref->next_space;
      //Update size because we have gotten some space
      *size -= pref->remaining_memory;
    }
  }
  
  //If size is still not 0 we get as much as we can from it or until size is 0
  if ( (*size != ((__myfs_offset_t) 0)) && (largest != NULL) ) {
    ptr = largest;
    //Check if the largest block can give everything that we are missing
    if (largest->remaining_memory >= *size) {
      //Check if we can make an s_Allocation object after getting size bytes from it
      if (largest->remaining_memory > *size + sizeof(s_Allocation)) {
        //Make the new s_Allocation object
        temp = ((void *) largest) + sizeof(size_t) + *size; //
        temp->remaining_memory = largest->remaining_memory - *size - sizeof(size_t);
        temp->next_space = largest->next_space;
        //Update before_largest pointer, it may be the LL or a s_Allocation
        if (before_largest == NULL) {
          //before_largest is the LL
          LL->first_space = largest_off + *size + sizeof(size_t);
        }
        else {
          //Update pointers to add temp list of free blocks
          before_largest->next_space = largest_off + *size + sizeof(size_t);
        }
        //Set original pref with final total size
        ptr->remaining_memory = *size;
      }
      //We can't make an s_Allocation object so we get everything
      else {
        //Use everything that the largest block have
        if (before_largest != NULL) {
          before_largest->next_space = largest->next_space;
        }
        else {
          //Because largest is the first block and we use everything, it means that the system have no memory left
          LL->first_space = ((__myfs_offset_t) 0);
        }
      }
      *size = ((__myfs_offset_t) 0);
    }
    //We couldn't got everything from the largest block so we get as much as we can from it
    else {
      //Update pointers
      if (before_largest == NULL) {
        //We had use everything and we still didn't got enough size
        return NULL;
      }
      else {
        before_largest->next_space = largest->next_space;
        //Update size
        *size -= largest->remaining_memory;
      }
    }
  }
  //We suppose to return the new block address if a new block outside the prefer one was needed
  return ((void *) ptr) + sizeof(size_t); //ptr was set to the largest which is at the size_t header so we resize
}


__myfs_inode_t *get_path(__myfs_handle_t handle, const char *path){
  __myfs_inode_t *node, *child;
  char *index, *__path, *name, file_name[MYFS_MAXIMUM_NAME_LENGTH];
  size_t size;
  
  if (handle->root_dir == (__myfs_offset_t) 0){
    struct timespec ts;
    clock_gettime(CLOCK_REALTIME, &ts);
    handle->root_dir = allocate_memory(handle, INODE_SIZE);
    __myfs_inode_t *root = (__myfs_inode_t *) offset_to_ptr(handle, handle->root_dir);
    root->name[0] = '/';
    root->name[1] = '\0';
    root->type = DIRECTORY;
    root->times[0] = ts;
    root->times[1] = ts;
    root->value.directory.number_children = (size_t) 0;
    root->value.directory.children = (__myfs_offset_t) 0;
  }
  
  node = (__myfs_inode_t *) offset_to_ptr(handle, handle->root_dir);
  
  if (strcmp("/\0", path) == 0){ // path is root directory
    return node;
  }
  
  __path = (char *) malloc((strlen(path)+1) * sizeof(char));
  if (__path == NULL) {
    return NULL;
  }
  strcpy(__path, path);
  name = __path + 1;
  index = strchr(name, '/');
  
  while (strlen(name) != 0){
    child = NULL; 
    
    if (index != NULL) {
      size = (size_t) (((void *) index) - ((void *) name));
      
    } else {
      size = (size_t) strlen(name);
    }
    strncpy(file_name, name, size);
    file_name[size] = '\0';
    
    for (size_t i = 0; i < node->value.directory.number_children; i++){
      child = (__myfs_inode_t *) offset_to_ptr(handle,
					       (node->value.directory.children + i * sizeof(__myfs_inode_t)));
      
      if (strcmp(child->name, file_name) == 0){
	node = child;
	break;
      }
    }
        
    memset(file_name, 0, size);
    if (node != child) { // path not found
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

/* End of helper functions */

/* Implements an emulation of the stat system call on the filesystem 
   of size fssize pointed to by fsptr. 
   
   If path can be followed and describes a file or directory 
   that exists and is accessable, the access information is 
   put into stbuf. 
   
   On success, 0 is returned. On failure, -1 is returned and 
   the appropriate error code is put into *errnoptr.

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
   st_atime
   st_mtime

*/
int __myfs_getattr_implem(void *fsptr, size_t fssize, int *errnoptr,
                          uid_t uid, gid_t gid,
                          const char *path, struct stat *stbuf) {
  
  __myfs_handle_t handle;
  __myfs_inode_t *node;
  
  handle = __myfs_get_handle(fsptr, fssize);
  
  if (handle == NULL) {
    errnoptr = EFAULT;
    return -1;
  }
  
  node = __myfs_path_resolve(handle, path);
  if (node == NULL) {
    *errnoptr = ENOENT;
    return -1;
  }
  
  memset(stbuf, 0, sizeof(struct stat));
  
  stbuf->st_uid = uid; 
  stbuf->st_gid = gid;
  stbuf->st_atim = node->times[0];
  stbuf->st_mtim = node->times[1];
  
  if (node->type == DIRECTORY) {
    stbuf->st_mode = S_IFDIR | 0755;
    stbuf->st_nlink = node->value.directory.number_children + ((size_t) 2);
  } else {
    stbuf->st_mode = S_IFREG | 0755;
    stbuf->st_size = node->value.file.size;
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
  __myfs_handle_t handle;
  __myfs_inode_t *node, *child;
  char **names;
  size_t size;
  
  handle = get_handle(fsptr, fssize);
  if (handle == NULL){
    *errnoptr = EFAULT;  
    return -1;
  }
  
  node = get_path(handle, path);
  if (node == NULL){
        *errnoptr = ENOENT;
        return -1;
  }
  
  size = node->value.directory.number_children;
  if (size == (size_t) 0){
    return 0;
  }
  
  names = (char **) calloc(size, sizeof(char *));
  if (names == NULL){
    *errnoptr = ENOMEM;
    return -1;
  }
  
  for (size_t i = 0; i < size; i++){
    child = ((__myfs_inode_t *) offset_to_ptr(handle,
					      (node->value.directory.children + i * sizeof(__myfs_inode_t))));
    
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
  __myfs_handle_t handle;
  __myfs_inode_t *node, *child;
  char *file_name, *dir_path;
  size_t dir_len, num_children;
  struct timespec ts;
    clock_gettime(CLOCK_REALTIME, &ts);
    //printf("MKNOD %s\n", path);
    
    handle = get_handle(fsptr, fssize);
    if (handle == NULL) {
      *errnoptr = EFAULT;  
      return -1;
    }
    
    if (INODE_SIZE > max_size(handle)){
      *errnoptr = ENOMEM;
      return -1;
    }
    
    file_name = strrchr(path, '/') + 1;
    dir_len = strlen(path) - strlen(file_name);
    if (strlen(file_name) >= MYFS_MAXIMUM_NAME_LENGTH) {
      *errnoptr = ENAMETOOLONG;
      return -1;
    }
    
    dir_path = (char *) malloc((dir_len+1) * sizeof(char));
    if (dir_path == NULL){
      *errnoptr = ENOMEM;
      return -1;
    }
    
    strncpy(dir_path, path, dir_len);
    dir_path[dir_len] = '\0';
    
    node = get_path(handle, dir_path);
    if (node == NULL) {
      free(dir_path);
      *errnoptr = ENOENT;
      return -1;
    }
    
    node->value.directory.number_children++;
    num_children = node->value.directory.number_children;
    
    if (num_children == 1) {
      node->value.directory.children = allocate_memory(handle, INODE_SIZE);
      if (node->value.directory.children == (__myfs_offset_t) 0 ) {
	*errnoptr = ENOMEM;
	return -1;
      }
    } else {
      node->value.directory.children = reallocate_memory(handle,
							 node->value.directory.children, num_children * INODE_SIZE);
      if (node->value.directory.children == (__myfs_offset_t) 0){
	*errnoptr = ENOMEM;
	return -1;
      }
    }
    
    child = (__myfs_inode_t *) offset_to_ptr(handle, (node->value.directory.children 
						      + (num_children-1) * INODE_SIZE)); 
    
    strcpy(child->name, file_name);
    //printf("Child name %s\n", child->name);
    child->type = REG_FILE;
    child->times[0] = ts;
    child->times[1] = ts;
    child->value.file.size = (size_t) 0;
    child->value.file.first_block = (__myfs_offset_t) 0;
    
    free(dir_path);
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
  
  __myfs_handle_t handle;
  __myfs_inode_t *file_node, *dir_node, *node;
  char *file_name, *dir_path;
  size_t dir_len;
  __myfs_file_block_t *prev, *file_block;
  
  handle = get_handle(fsptr, fssize);
  if (handle == NULL){
    *errnoptr = EFAULT;  
    return -1;
  }
  
  file_node = get_path(handle, path);
  if (file_node == NULL){
    *errnoptr = ENOENT;
    return -1;
  }
  
  if (file_node->type == DIRECTORY){
    *errnoptr = EISDIR;
    return -1;
  }

  file_name = strrchr(path, '/') + 1;
  dir_len = strlen(path) - strlen(file_name);
  
  dir_path = (char *) malloc((dir_len+1) * sizeof(char));
    if (dir_path == NULL){
      *errnoptr = ENOMEM;
      return -1;
    }
    strncpy(dir_path, path, dir_len);
    dir_path[dir_len] = '\0';
    
    dir_node = get_path(handle, dir_path);
    if (dir_node == NULL) {
      free(dir_path);
      *errnoptr = ENOENT;
      return -1;
    }

    for (size_t i = 0; i < dir_node->value.directory.number_children; i++){
      node = (__myfs_inode_t *) offset_to_ptr(handle,
					      dir_node->value.directory.children + i * INODE_SIZE);
      
      if (strcmp(node->name, file_name) == 0){
            break;
      }
    }
    
    for (file_block = (__myfs_file_block_t *) offset_to_ptr(handle,
                node->value.file.first_block), prev = NULL; file_block != NULL;
	 prev = file_block, file_block = (__myfs_file_block_t *) offset_to_ptr(handle,
									       file_block->next)){
      
        //free_memory(handle, file_block->data);
      if (file_block->data != (__myfs_offset_t) 0)
	file_block->data = reallocate_memory(handle,
					     file_block->data, (size_t) 0);
      
      if (prev != NULL){
            //free_memory(handle, ptr_to_offset((void *) prev, handle));
	prev = (__myfs_file_block_t *) reallocate_memory(handle,
							 ptr_to_offset((void *) prev, handle), (size_t) 0);
      } 
    }
    
    if (prev != NULL){
      //free_memory(handle, ptr_to_offset((void *) prev, handle));
      prev = (__myfs_file_block_t *) reallocate_memory(handle,
						       ptr_to_offset((void *) prev, handle), (size_t) 0);
    }

    node->value.file.size = (size_t) 0;
    
    if (dir_node->value.directory.number_children > 1) {
      memcpy((void *) node, offset_to_ptr(handle, (dir_node->value.directory.children 
						   + (dir_node->value.directory.number_children - 1) * INODE_SIZE)),
	     INODE_SIZE);
    }
    
    dir_node->value.directory.number_children--;
    dir_node->value.directory.children = reallocate_memory(handle,
							   dir_node->value.directory.children, (dir_node->value.directory.number_children
												* INODE_SIZE));
    
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
  __myfs_handle_t handle;
  __myfs_inode_t *file_node, *dir_node, *node;
  char *dir_name, *dir_path;
  size_t dir_len;
  
  handle = get_handle(fsptr, fssize);
  if (handle == NULL){
    *errnoptr = EFAULT;  
    return -1;
  }
  
  file_node = get_path(handle, path);
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
  
  dir_node = get_path(handle, dir_path);
  if (dir_node == NULL){
    free(dir_path);
    *errnoptr = ENOENT;
    return -1;
  }
  
  
  for (size_t i = 0; i < dir_node->value.directory.number_children; i++){
    node = (__myfs_inode_t *) offset_to_ptr(handle,
					    dir_node->value.directory.children + i * INODE_SIZE);
    
    if (strcmp(node->name, dir_name) == 0){
      break;
    }
  }
  
  if (dir_node->value.directory.number_children > 1) {
    memcpy((void *) node, offset_to_ptr(handle, (dir_node->value.directory.children 
						 + (dir_node->value.directory.number_children - 1) * INODE_SIZE)),
	   INODE_SIZE);
  }
  
  dir_node->value.directory.number_children--;
  dir_node->value.directory.children = reallocate_memory(handle,
							 dir_node->value.directory.children, (dir_node->value.directory.number_children
											      * INODE_SIZE));
  
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
  __myfs_handle_t handle;
  __myfs_inode_t *node, *child, *dir_node;
  char *dir_name, *dir_path;
  size_t dir_len, num_children;
  struct timespec ts;
  clock_gettime(CLOCK_REALTIME, &ts);
  
  handle = get_handle(fsptr, fssize);
  if (handle == NULL){
    *errnoptr = EFAULT;  
    return -1;
  }
  
  if (INODE_SIZE > max_size(handle)){
    *errnoptr = ENOMEM;
    return -1;
  }
  
  dir_node = get_path(fsptr, path);
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
  
  node = get_path(handle, dir_path);
  if (node == NULL){
    free(dir_path);
    *errnoptr = ENOENT;
    return -1;
  }
  
  node->value.directory.number_children++;
  num_children = node->value.directory.number_children;
  
  if (num_children == 1) {
    node->value.directory.children = allocate_memory(handle, INODE_SIZE);
    if (node->value.directory.children == (__myfs_offset_t) 0){
      *errnoptr = ENOMEM;
      return -1;
    }
  }
  
  else {
    node->value.directory.children = reallocate_memory(handle,
						       node->value.directory.children, num_children * INODE_SIZE);
    if (node->value.directory.children == (__myfs_offset_t) 0){
      *errnoptr = ENOMEM;
      return -1;
    }
  }
  
  child = (__myfs_inode_t *) offset_to_ptr(handle, (node->value.directory.children 
						    + (num_children-1) * INODE_SIZE)); 
  
  strcpy(child->name, dir_name);
  child->type = DIRECTORY;
  child->times[0]= ts;
  child->times[1] = ts;
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
  __myfs_handle_t handle;
  __myfs_inode_t *from_file, *from_dir, *to_dir;
  char *from_file_name, *to_file_name, *from_dir_name, *to_dir_name;
  size_t from_dir_len, to_dir_len;
  
  //printf("RENAME %s to %s\n", from, to);
  if (strcmp(from , to) == 0)
    return 0;
  
  handle = get_handle(fsptr, fssize);
  if (handle == NULL){
    *errnoptr = EFAULT;  
    return -1;
  }
  
  from_file = get_path(handle, from);
  if (from_file == NULL){
    *errnoptr = ENOENT;
    return -1;
  }
  
  to_file_name = strrchr(to, '/') + 1;
  to_dir_len = strlen(to) - strlen(to_file_name);
  from_file_name = strrchr(from, '/') + 1;
  from_dir_len = strlen(from) - strlen(from_file_name);
  
  if (strlen(to_file_name) >= MYFS_MAXIMUM_NAME_LENGTH){
    *errnoptr = ENAMETOOLONG;
    return -1;
  }
  
  to_dir_name = (char *) malloc((to_dir_len + 1) * sizeof(char));
  from_dir_name = (char *) malloc((from_dir_len+1) * sizeof(char));
  if (from_dir_name == NULL || to_dir_name == NULL){
    *errnoptr = ENOMEM;
    return -1;
  }
  
  strncpy(from_dir_name, from, from_dir_len);
  from_dir_name[from_dir_len] = '\0';
  
  strncpy(to_dir_name, to, to_dir_len);
  to_dir_name[to_dir_len] = '\0';
  
  from_dir = get_path(handle, from_dir_name);
  if (from_dir == NULL){
    free(from_dir_name);
    free(to_dir_name);
    *errnoptr = ENOENT;
    return -1;
  }
  
  to_dir = get_path(handle, to_dir_name);
  if (to_dir == NULL){
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
  if (to_dir->value.directory.children == (__myfs_offset_t) 0){
    to_dir->value.directory.children = allocate_memory(handle,
						       (to_dir->value.directory.number_children * INODE_SIZE));
  }
  
    else{
      to_dir->value.directory.children = reallocate_memory(handle, 
            to_dir->value.directory.children, (to_dir->value.directory.number_children
					       * INODE_SIZE));
    }
  
  // copy file from the "from path" to the "to path"
  memmove(offset_to_ptr(handle, (to_dir->value.directory.children +
				 (to_dir->value.directory.number_children - 1) * INODE_SIZE)),
	  (void *) from_file, INODE_SIZE);
  
  // delete the file from the "from path"
  /*
    if (__myfs_unlink_implem(fsptr, fssize, errnoptr, from) == -1)
    return -1;
  */
  
  from_file->value.file.size = (size_t) 0;
  if (from_dir->value.directory.number_children > 1){
    memmove((void *) from_file, offset_to_ptr(handle,
					      (from_dir->value.directory.children
					       + (from_dir->value.directory.number_children - 1) * INODE_SIZE)),
	    INODE_SIZE);
  }
  
  from_dir->value.directory.number_children--;
  from_dir->value.directory.children = reallocate_memory(handle,
							 from_dir->value.directory.children, (from_dir->value.directory.number_children
											      * INODE_SIZE));
  
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
  __myfs_handle_t handle; 
  __myfs_inode_t *node;
  __myfs_file_block_t *file_block, *prev;
  off_t new_offset;
  
  //printf("TRUNCATE %s, offset %ld\n", path, offset);
  
  handle = get_handle(fsptr, fssize);
  if (handle == NULL){
    *errnoptr = EFAULT;  
    return -1;
  }
  
  node = get_path(handle, path);
  if (node == NULL){
    *errnoptr = ENOENT;
    return -1;
  }
  
  if (node->type == DIRECTORY){ // file is a directory
    *errnoptr = EISDIR;
    return -1;
  }
  
  if (node->value.file.size == (size_t) 0){
    if ((offset + (size_t) sizeof(__myfs_file_block_t)) > (off_t) max_size(handle)){
      *errnoptr = ENOMEM;
      return -1;
    }
    
    node->value.file.first_block = allocate_memory(handle, (size_t) sizeof(__myfs_file_block_t));
    
    if (node->value.file.first_block == (__myfs_offset_t) 0) {
      *errnoptr = ENOMEM;
      return -1;
    }
    
    file_block = (__myfs_file_block_t *) offset_to_ptr(handle,
						       node->value.file.first_block);
    node->value.file.size = offset;
  }
  
  else if (offset > node->value.file.size){
    if ((offset+(size_t) sizeof(__myfs_file_block_t)) > (off_t) max_size(handle)){
      *errnoptr = ENOMEM;
      return -1;
    }
    for (file_block = (__myfs_file_block_t *) offset_to_ptr(handle,
							    node->value.file.first_block), prev = NULL; file_block != NULL;
	 prev = file_block, file_block = (__myfs_file_block_t *) offset_to_ptr(handle,
									       file_block->next));
    
    prev->next = allocate_memory(handle, (size_t) sizeof(__myfs_file_block_t));
  if (prev->next == (__myfs_offset_t) 0) {
    *errnoptr = ENOMEM;
    return -1;
  }
  
  file_block = (__myfs_file_block_t *) offset_to_ptr(handle, prev->next);
  node->value.file.size = offset;
} 

 else{
        node->value.file.size = offset;
        new_offset = offset; 
        for (file_block = (__myfs_file_block_t *) offset_to_ptr(handle,
								node->value.file.first_block); file_block != NULL;
	     file_block = (__myfs_file_block_t *) offset_to_ptr(handle,
								file_block->next)){
	  
	  if (new_offset > file_block->size){
	    new_offset -= file_block->size;
	  } else {
                break;
	  }
        }
	
        if (file_block == NULL){
	  //printf("This shouldn't happen\n");
	  return -1;
        } 
        
        file_block->data = reallocate_memory(handle, file_block->data, new_offset);
        file_block->size = new_offset;
        prev = NULL;
        file_block = (__myfs_file_block_t *) offset_to_ptr(handle, file_block->next);
        while (file_block != NULL){
	  if (prev != NULL){
	    prev = (__myfs_file_block_t *) reallocate_memory(handle, ptr_to_offset((void *)
										   prev, handle), (size_t) 0);
	  }
	  
	  free_memory(handle, file_block->data);
	  prev = file_block;
	  file_block = (__myfs_file_block_t *) offset_to_ptr(handle,
							     file_block->next);
        }
            if (prev != NULL){
	      free_memory(handle, ptr_to_offset((void *) prev, handle));
            }
	    return 0;
 }

file_block->next = (__myfs_offset_t) 0;
file_block->data = allocate_memory(handle, offset);
if (file_block->data == (__myfs_offset_t) 0){
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
  __myfs_handle_t handle; 
  __myfs_inode_t *node;
  
  
  handle = get_handle(fsptr, fssize);
  if (handle == NULL) {
    *errnoptr = EFAULT;  
    return -1;
  }
  
  node = get_path(handle, path);
  if (node == NULL){
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
  __myfs_handle_t handle; 
  __myfs_inode_t *node;
  __myfs_file_block_t *file_block;
  size_t new_size;
  off_t new_offset;
  int num_bytes = 0;
  
  handle = get_handle(fsptr, fssize);
    if (handle == NULL) {
      *errnoptr = EFAULT;  
      return -1;
    }
    
    node = get_path(handle, path);
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
    for (file_block = (__myfs_file_block_t *) offset_to_ptr(handle,
							    node->value.file.first_block);
	 file_block != NULL;
	 file_block = (__myfs_file_block_t *) offset_to_ptr(handle,
							    file_block->next)) {
      
      if (new_offset > file_block->size) {
	new_offset -= file_block->size;
      }
      else{
	break;
      }
    }
    
    while (file_block != NULL){
      if (((size_t) new_offset + new_size) < file_block->size){
	memcpy(buf + num_bytes, offset_to_ptr(handle, ((size_t) new_offset
						       + file_block->data)), new_size);
	num_bytes += (int) new_size;
	break;
      }
      
      else{
	new_size -= (file_block->size - new_offset);     
	memcpy(buf + num_bytes, offset_to_ptr(handle, ((size_t) new_offset
						       + file_block->data)), 
	       (file_block->block_size - new_offset));
	
	num_bytes += (int) (file_block->size - new_offset);
      }
      
      if (new_offset != (off_t) 0)
	new_offset = 0;
      
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
  __myfs_handle_t handle; 
  __myfs_inode_t *node;
  __myfs_file_block_t *file_block;
  int num_bytes;
  
  
  handle = get_handle(fsptr, fssize);
  if (handle == NULL) {
    *errnoptr = EFAULT;  
        return -1;
  }
  
  node = get_path(handle, path);
  if (node == NULL) {
    *errnoptr = ENOENT;
    return -1;
  }
  
  if (node->type == DIRECTORY) {
    *errnoptr = EISDIR;
    return -1;
  }
  
  if (node->value.file.size == (size_t) 0) {
    node->value.file.first_block = allocate_memory(handle, (size_t) sizeof(__myfs_file_block_t));
    
    if (node->value.file.first_block == (size_t) 0) {
      *errnoptr = ENOMEM;
      return -1;
    }
  }
  
  if (offset > node->value.file.size)
    return 0;
  
  file_block = (__myfs_file_block_t*) offset_to_ptr(handle, node->value.file.first_block);
  
  if (node->value.file.size == (size_t) 0) {
    file_block->data = allocate_memory(handle, size);
    
    if (file_block->data == (size_t) 0){
      *errnoptr = ENOMEM;
      return -1;
    }
    file_block->size = size;
    file_block->next = (__myfs_offset_t) 0;
    node->value.file.size = size;
  }
  
  else if (((size_t) offset + size) > node->value.file.size){
    for (file_block = (__myfs_file_block_t *) offset_to_ptr(handle,
							    node->value.file.first_block);
	 file_block->next != (__myfs_offset_t) 0;
	 file_block = (__myfs_file_block_t *) offset_to_ptr(handle,
							    file_block->next));
    
    file_block->next = allocate_memory(handle, (size_t) sizeof(__myfs_file_block_t));
    
    if (file_block->next == (size_t) 0) {
      *errnoptr = ENOMEM;
      return -1;
    }
    
    file_block = (__myfs_file_block_t *) offset_to_ptr(handle, 
						       file_block->next);
    
    file_block->data = allocate_memory(handle, size);
    
    if (file_block->data == (size_t) 0){
      *errnoptr = ENOMEM;
      return -1;
    }
    file_block->next = (__myfs_offset_t) 0;
    
    file_block->size = size;
    node->value.file.size += size;
  }
  
  num_bytes = (int) size;
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
  __myfs_handle_t handle;
  __myfs_inode_t *node;
  // get handle 
  handle = __myfs_get_handle(fsptr, fssize);
  if (handle == NULL) {
    *errnoptr = EFAULT;
    return -1;
  }
  // get path
  node = __myfs_path_resolve(fsptr, path);
  if (node == NULL) {
    *errnoptr = ENOENT;
    return -1;
  }
  /* change node times in struct timespec times[2] */
  node->times[0] = ts[0]; // access | st_atim
  node->times[1] = ts[1]; // modification | st_mtim
  
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

/* 
  Get handle 
  how many blocks are our size 1024(1 byte, can use other sizes)
  blocks = systemsize/ size 
  free blocks 
  available blocks 
  name max length */ 
int __myfs_statfs_implem(void *fsptr, size_t fssize, int *errnoptr,
                         struct statvfs* stbuf) {
  
  __myfs_handle_t handle;
  handle = __myfs_get_handle(fsptr,fssize);
  
  // size_t block_size = MYFS_BLOCK_SIZE ((size_t) 1024); 
  
  if (handle == NULL) {
    *errnoptr = EFAULT;
    return -1;
  }
  
  /* How many blocks are our size? - 1024 | (1 byte, can use other defined sizes though)
     Iterates over free space offsets and computes the total amount of free memory left  */
  memset(stbuf, 0, sizeof(statvfs));
  
  /* Block size is standard size of 1024
     blocks = BLOCK_SIZE / SIZE */ 
  stbuf->f_bsize = MYFS_BLOCK_SIZE; 
  stbuf->f_blocks = ((fsblkcnt_t) (handle->size / MYFS_BLOCK_SIZE));
  stbuf->f_bfree = ((fsblkcnt_t) (free_space(handle) / MYFS_BLOCK_SIZE));
  stbuf->f_bavail = stbuf->f_bfree;
  stbuf->f_namemax = (u_long) MYFS_MAXIMUM_NAME_LENGTH; // 256 characters 
  return 0;
}

