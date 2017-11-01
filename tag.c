#include <stdio.h>
#include <cache.h>

// gcc tag.c -o tag -lm

int main(int argc, const char* argv[])
{
	unsigned int size = 16;
	unsigned int assoc = 4;
	unsigned int bsize = 8;
	unsigned int mem_latency = 20;
	
	unsigned int address = 12345678;
	// 00000000 10111100 01100001 01001110
	
	struct cache_t *cache;
	cache = cache_create(size, bsize, assoc, mem_latency);
	
	cycle_number = cache_access(cache, address, 1);
	printf("read: %u", cycle_number);
	cycle_number = cache_access(cache, address, 0);
	printf("write: %u", cycle_number);
}
