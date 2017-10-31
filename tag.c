#include <stdio.h>
#include <inttypes.h>
#include <arpa/inet.h>
#include <math.h>

int main(int argc, char **argv)
{
	unsigned int blocksize = argv[1];
	unsigned int nsets = argv[2];
	unsigned long address = argv[3];
	
	unsigned int offset_size = log2(blocksize);
	unsigned int index_size = log2(nsets);
	unsigned int tag_size = 32 - (offset_size + index_size);
	
	unsigned int offset_lshift = tag_size + index_size;
	unsigned int offset_rshift = 32 - offset_size;
	unsigned int index_lshift = tag_size;
	unsigned int index_rshift = 32 - index_size;
	unsigned int tag_rshift = offset_size + index_size;

	unsigned int offset = address << offset_lshift;
	offset = offset >> offset_rshift;
	unsigned int index = address << index_lshift;
	index = index >> index_rshift;
	unsigned int tag = address >> tag_rshift;
	
	printf("blocksize: %u\nsets: %u\naddress: %u\ntag: %u\nindex: %u\n offset: %u", blocksize, nsets, address, tag, index, offset);
}