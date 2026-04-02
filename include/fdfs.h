#ifndef FDFS_H_INCLUDED
#define FDFS_H_INCLUDED

#include <stdint.h>

int fdfs_make(const char *image_path, uint64_t total_bytes);
int fdfs_open(const char *image_path, int cache_size);
int fdfs_close(void);

#endif
