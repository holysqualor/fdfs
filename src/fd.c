#include <stdio.h>
#include <stdint.h>

uint32_t fd32(uint32_t idx, uint32_t nblocks) {
	if(!idx)
		return 0;
	uint32_t p = 31 - __builtin_clz(idx);
	uint32_t d = nblocks >> p;
	return (d >> 1) + d * (idx ^ (1U << p));
}

int main() {
	#define size 16

	char disk[size + 1] = {};
	for(uint32_t i = 0; i < size; i++)
		disk[fd32(i, size)] = 'A' + i;
	printf("%s\n", disk);
	return 0;
}