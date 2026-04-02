#define _GNU_SOURCE

#include <stdio.h>
#include <stdlib.h>
#include <string.h>
#include <unistd.h>
#include <fcntl.h>
#include "fdfs.h"

#define FDFS 0xaa100f9c277e9b01

typedef struct {
	uint64_t length, next;
} fdfs_extent_s;

typedef struct {
	uint64_t block;
	int dirty;
} fdfs_page_s;

static struct {
	fdfs_extent_s *map;
	fdfs_page_s *pages;
	void *cache;
	uint64_t size, begin;
	int cache_size, image;
} Disk = {};

static const uint64_t block_size = 4096;

int fdfs_make(const char *image_path, uint64_t total_bytes) {
	if(!image_path || total_bytes < block_size)
		return -1;
	total_bytes = 1LL << (63 - __builtin_clzll(total_bytes));
	uint64_t disk_blocks = total_bytes / block_size;
	uint64_t begin = disk_blocks * sizeof(fdfs_extent_s);
	if(begin % block_size)
		begin = begin / block_size * block_size + block_size;
	begin += block_size;
	uint64_t header_size = begin + block_size;
	int image = open(image_path, O_WRONLY | O_CREAT | O_DIRECT, S_IRUSR | S_IWUSR | S_IRGRP | S_IWGRP | S_IROTH | S_IWOTH);
	if(image < 0 || ftruncate(image, total_bytes + begin) == -1) {
		if(image > -1)
			close(image);
		return -1;
	}
	uint64_t *header;
	if(posix_memalign((void**)&header, block_size, header_size)) {
		close(image);
		return -1;
	}
	memset(header, 0, header_size);
	header[0] = FDFS;
	header[1] = begin;
	header[2] = disk_blocks;
	((fdfs_extent_s*)((char*)header + block_size))->length = 1;
	uint64_t bytes = write(image, header, header_size);
	free(header);
	close(image);
	return bytes == header_size ? 0 : -1;
}

int fdfs_open(const char *image_path, int cache_size) {
	if(!image_path || cache_size <= 0 || Disk.cache_size)
		return -1;
	Disk.image = open(image_path, O_RDWR | O_DIRECT);
	if(Disk.image == -1) {
		Disk.image = 0;
		return -1;
	}
	uint64_t *header = NULL;
	if(posix_memalign((void**)&header, block_size, block_size)
	|| read(Disk.image, header, block_size) != block_size || header[0] != FDFS) {
		free(header);
		return fdfs_close() - 1;
	}
	Disk.begin = header[1];
	Disk.size = header[2];
	free(header);
	uint64_t map_size = Disk.begin - block_size;
	if(posix_memalign((void**)&Disk.map, block_size, map_size)
	|| posix_memalign((void**)&Disk.cache, block_size, block_size * cache_size)
	|| !(Disk.pages = malloc(sizeof(fdfs_page_s) * cache_size))
	|| read(Disk.image, Disk.map, map_size) != map_size)
		return fdfs_close() - 1;
	memset(Disk.pages, 0, sizeof(fdfs_page_s) * cache_size);
	Disk.cache_size = cache_size;
	return 0;
}

int fdfs_close(void) {
	if(Disk.cache_size) {
		for(int i = 0; i < Disk.cache_size; i++) {
			if(Disk.pages[i].dirty)
				pwrite(Disk.image, (uint8_t*)Disk.cache + block_size * i, block_size, Disk.begin + Disk.pages[i].block * block_size);
		}
	}
	free(Disk.map);
	free(Disk.cache);
	free(Disk.pages);
	if(Disk.image > 0)
		close(Disk.image);
	memset(&Disk, 0, sizeof(Disk));
	return 0;
}
