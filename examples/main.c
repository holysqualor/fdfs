#include <stdio.h>

#include "fdfs.h"

int main() {
	const char *image = "disk.fdfs";
	int cache_size = 16;
	if(fdfs_open(image, cache_size)) {
		if(fdfs_make(image, 4096 * 128) && fdfs_open(image, cache_size)) {
			printf("fatal error\n");
			return 1;
		}
	}
	printf("success\n");
	fdfs_close();
	return 0;
}
