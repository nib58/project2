#include <stdio.h>
#include "cache.h"

int main(int argc, const char* argv[])
{
	unsigned int size = 16;
	unsigned int assoc = 4;
	unsigned int bsize = 8;
	unsigned int mem_latency = 20;
	
	
	unsigned int address1 = 12345678;
	// 00000000101111000110000101001110
	// 16, 2, 8
	// 0000000010111100011 | 0000101001 | 110
	unsigned int address2 = 12337486;
	// 00000000101111000100000101001110
	// 16, 2, 8
	// 0000000010111100010 | 0000101001 | 110
	unsigned int address3 = 12329294;
	// 00000000101111000010000101001110
	// 16, 2, 8
	// 0000000010111100001 | 0000101001 | 110
	unsigned int address4 = 12321102;
	// 00000000101111000000000101001110
	// 16, 2, 8
	// 0000000010111100000 | 0000101001 | 110
	unsigned int address5 = 12353870;
	// 00000000101111001000000101001110
	// 16, 2, 8
	// 0000000010111100100 | 0000101001 | 110

	unsigned int cycle_number = 0;
	
	struct cache_t *cache;
	cache = cache_create(size, bsize, assoc, mem_latency);
	
	cycle_number = cache_access(cache, address1, 1);
	printf("WRITE: %u\n", cycle_number);
	cycle_number = cache_access(cache, address2, 1);
	printf("WRITE: %u\n", cycle_number);
	cycle_number = cache_access(cache, address3, 1);
	printf("WRITE: %u\n", cycle_number);
	cycle_number = cache_access(cache, address4, 1);
	printf("WRITE: %u\n", cycle_number);
	cycle_number = cache_access(cache, address1, 1);
	printf("WRITE: %u\n", cycle_number);
	cycle_number = cache_access(cache, address2, 1);
	printf("WRITE: %u\n", cycle_number);
	cycle_number = cache_access(cache, address3, 1);
	printf("WRITE: %u\n", cycle_number);
	cycle_number = cache_access(cache, address1, 1);
	printf("WRITE: %u\n", cycle_number);
	cycle_number = cache_access(cache, address2, 1);
	printf("WRITE: %u\n", cycle_number);
	cycle_number = cache_access(cache, address1, 1);
	printf("WRITE: %u\n", cycle_number);
	// Replace
	cycle_number = cache_access(cache, address5, 1);
	printf("WRITE: %u\n", cycle_number);

	return(0);
}
