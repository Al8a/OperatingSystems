#ifndef __MYFS_IMPL__
#define __MYFS_IMPL__

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


/* Macros -------------------- */
#define MYFS_MAXIMUM_NAME_LENGTH (256)
#define MYFS_BLOCK_SIZE ((size_t) 1024)
//#define MYFS_BLOCK_SIZE ((size_t) (4096))
#define MYFS_STATIC_PATH_BUF_SIZE (8192)
#define MYFS_MAGIC (UINT32_C(0xcafebabe)) 
#define MYFS_TRUNCATE_SMALL_ALLOCATE ((size_t) 512)
#define INODE_SIZE ((size_t) sizeof(__myfs_inode_t))

typedef size_t __myfs_offset_t; 
typedef unsigned int u_int;
typedef unsigned long u_long;


/* Structs Declarations -------------------- */
/*  Handler structure "Super-block" */ 
typedef struct __myfs_handle_struct_t *__myfs_handle_t;
struct __myfs_handle_struct_t {
  uint32_t magic;
  __myfs_offset_t free_memory;
  __myfs_offset_t root_dir;
  size_t size;
};


/* File Block Structure */ 
typedef struct __myfs_file_block_struct_t __myfs_file_block_t;
struct __myfs_file_block_struct_t {
  size_t size;
  size_t allocated;
  __myfs_offset_t next;
  __myfs_offset_t data;
};


/* Memory block structure */
typedef struct __myfs_memory_block_struct_t *__myfs_mem_block_t;
struct __myfs_memory_block_struct_t {
  size_t size;
  size_t user_size;
  __myfs_offset_t next; 
};


/* inode struct */
typedef struct __myfs_inode_struct_t __myfs_inode_t;
struct __myfs_inode_struct_t {
  __myfs_inode_type_t type;
  char name[MYFS_MAXIMUM_NAME_LENGTH];
  struct timespec times[2];
  /*  st_atim | times[0]: time of last access 
      st_mtim | times[1]: time of last modification 
      st_ctim | times[2]: time of last status change <- Doesnt need to be implemented */
  union {
    __myfs_inode_file_t file;
    __myfs_inode_directory_t directory;
  } value;
};


/* inode file */
typedef struct __myfs_inode_file_struct_t __myfs_inode_file_t;
struct __myfs_inode_file_struct_t {
  size_t size;
  __myfs_offset_t first_block;
};


/* inode directory  */
typedef struct __myfs_inode_directory_struct_t __myfs_inode_directory_t;
struct __myfs_inode_directory_struct_t {
  size_t number_children;
  __myfs_offset_t children;
};


/* types for inode entries */
typedef enum __myfs_inode_type_enum_t __myfs_inode_type_t;
enum __myfs_inode_type_enum_t {
  DIRECTORY,
  REG_FILE
};
/* End struct definitions */



/* Helper function definitions */
static inline void * offset_to_ptr(void *fsptr, __myfs_offset_t off);
static inline __myfs_offset_t ptr_to_offset(void *fsptr, void *ptr);

static __myfs_handle_t  __myfs_get_handle(void *fsptr, size_t size);
static __myfs_inode_t *__myfs_path_resolve(__myfs_handle_t handle, const char *path);
static __myfs_inode_t * __myfs_path_resolve_one_step(__myfs_handle_t handle, __myfs_inode_t *curr);

static void __myfs_set_filename(char *dst, const char *src);
static void __myfs_set_curr_time(__myfs_inode_t *node, int modified_flag);



/* Memory allocation methods -------------- */
/* Struct definitons */
typedef struct s_Allocation {
  size_t remaining_memory;
  __myfs_offset_t next_space;
} s_allocation ;

typedef struct mem_header {
  __myfs_offset_t first_space;
} linkedList;

void *__malloc_impl(void *fsptr, void *ptr, size_t *size);
void *__realloc_impl(void *fsptr, void *ptr, size_t *size);
void *__calloc_impl(void *fsptr, size_t numMemBlocks, size_t *size);     
void __free_impl(void *fsptr, void *ptr);



/* 13 methods to implement */
int __myfs_getattr_implem(void *fsptr, size_t fssize, int *errnoptr,uid_t uid, gid_t gid, const char *path, struct stat *stbuf);
int __myfs_readdir_implem(void *fsptr, size_t fssize, int *errnoptr, const char *path, char ***namesptr);
int __myfs_mknod_implem(void *fsptr, size_t fssize, int *errnoptr, const char *path);
int __myfs_unlink_implem(void *fsptr, size_t fssize, int *errnoptr, const char *path);
int __myfs_rmdir_implem(void *fsptr, size_t fssize, int *errnoptr, const char *path);
int __myfs_mkdir_implem(void *fsptr, size_t fssize, int *errnoptr, const char *path);
int __myfs_rename_implem(void *fsptr, size_t fssize, int *errnoptr, const char *from, const char *to);
int __myfs_truncate_implem(void *fsptr, size_t fssize, int *errnoptr, const char *path, off_t offset);
int __myfs_open_implem(void *fsptr, size_t fssize, int *errnoptr, const char *path);
int __myfs_read_implem(void *fsptr, size_t fssize, int *errnoptr, const char *path, char *buf, size_t size, off_t offset);
int __myfs_write_implem(void *fsptr, size_t fssize, int *errnoptr, const char *path, const char *buf, size_t size, off_t offset);
int __myfs_utimens_implem(void *fsptr, size_t fssize, int *errnoptr, const char *path, const struct timespec ts[2]);
int __myfs_statfs_implem(void *fsptr, size_t fssize, int *errnoptr, struct statvfs* stbuf);
#endif