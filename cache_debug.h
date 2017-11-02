#include <stdlib.h>
#include <stdio.h>
#include <math.h>

struct cache_blk_t
{
    unsigned long tag;
    char valid;
    char dirty;
    unsigned LRU;
};

struct cache_t
{
    // The cache is represented by a 2-D array of blocks.
    // The first dimension of the 2D array is "nsets" which is the number of sets (entries)
    // The second dimension is "assoc", which is the number of blocks in each set.
    int nsets;                      // number of sets
    int blocksize;                  // block size
    int assoc;                      // associativity
    int mem_latency;                // the miss penalty
    struct cache_blk_t **blocks;    // a pointer to the array of cache blocks
};

int logarithm(int x)
{
	int count = 0;
	while(x > 1)
	{
		x = x / 2;
		count++;
	}
	return count;
}

struct cache_t * cache_create(int size, int blocksize, int assoc, int mem_latency)
{
    int i;
    int nblocks = (size * 1024) / blocksize;    // number of blocks in the cache
    int nsets = nblocks / assoc;                // number of sets (entries) in the cache
    struct cache_t *C = (struct cache_t *)calloc(1, sizeof(struct cache_t));

    C->nsets = nsets;
    C->blocksize = blocksize;
    C->assoc = assoc;
    C->mem_latency = mem_latency;

    C->blocks = (struct cache_blk_t **)calloc(nsets, sizeof(struct cache_blk_t *));

    for(i = 0; i < nsets; i++)
    {
		C->blocks[i] = (struct cache_blk_t *)calloc(assoc, sizeof(struct cache_blk_t));
    }

    return C;
}

int cache_access(struct cache_t *cp, unsigned long address, int access_type)
{
    //
    // Based on "address", determine the set to access in cp and examine the blocks
    // in the set to check hit/miss and update the golbal hit/miss statistics
    // If a miss, determine the victim in the set to replace (LRU).
    //
    // The function should return the hit_latency, which is 0, in case of a hit.
    // In case of a miss, the function should return mem_latency if no write back is needed.
    // If a write back is needed, the function should return 2*mem_latency.
    // access_type (0 for a read and 1 for a write) should be used to set/update the dirty bit.
    // The LRU field of the blocks in the set accessed should also be updated.
	
    int penalty = cp->mem_latency;
	
	// Setting Sizes and Computing Values
	unsigned int offset_size = logarithm(cp->blocksize);
	unsigned int index_size = logarithm(cp->nsets);
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
	
	// Check Blocks
	int i;
	
	int hit = 0;
	int set_size = cp->assoc;
	int block_index = -1;
	int LRU_value = 0;
	int LRU_comp = 0;
	
	struct cache_blk_t * block;
	struct cache_blk_t * LRU_block;
	struct cache_blk_t * set = cp->blocks[index];
	
	//int LRU[set_size];
	//int LRU_val = 0;
	
	printf("------------------------------------------------------------\n");
	printf("ADDRESS: %lu\n", address);
	printf("TAG: %u:%u\n", tag, tag_size);
	printf("INDEX: %u:%u\n", index, index_size);
	printf("OFFSET: %u:%u\n", offset, offset_size);
	printf("------------------------------------------------------------\n");
	printf("INITIAL SET:\n");
	
	for(i = 0; i < set_size; i++)
	{
		block = &set[i];
		
		printf("--------------------\n");
		printf("BLOCK: %u\n", i);
		printf("B:Valid %u\n", block->valid);
		printf("B:Dirty %u\n", block->dirty);
		printf("B:Tag %lu\n", block->tag);
		printf("B:LRU %u\n", block->LRU);
		
		if (block->valid == 1)
		{
			if (block->tag == tag)
			{	// HIT
				printf("HIT!\n");
				block_index = i;
				hit = 1;
			}
			else
			{
				printf("MISS!\n");
			}
		}
		else
		{	// MISS
			printf("INITIAL MISS!\n");
			block->LRU = 1;
		}
	}
	
	printf("------------------------------------------------------------\n");
	
	printf("INITIAL TARGET: %u\n", block_index);
	
	if (hit == 1)
	{	// HIT		
		penalty = 0;
		printf("FINAL: HIT\n");
	}
	else
	{	// MISS
		// LRU Get
		
		// block = LRU;
		for(i = 0; i < set_size; i++)
		{
			LRU_block = &set[i];
			LRU_value = LRU_block->LRU;
			if (LRU_value > LRU_comp)
			{
				LRU_comp = LRU_value;
				block = LRU_block;
				block_index = i;
			}
		}
		
		if (block->dirty == 1)
		{	// Write Back
			penalty = penalty * 2;
		}
		else
		{	// No Write Back
			// penalty = penalty;
		}
		
		if (access_type == 0)
		{	// READ
			// Read Resets Dirty Bit On Miss
			block->dirty = 0;
		}
		
		printf("FINAL: MISS\n");
	}

	if (access_type == 1)
	{	// WRITE
		// Write Always Sets Dirty Bit	
		block->dirty = 1;
	}
	
	printf("FINAL TARGET: %u\n", block_index);
	printf("------------------------------------------------------------\n");
	printf("FINAL SET:\n");
	
	// For Whatever Block Was Accessed, This Is True

	// LRU Update
	for(i = 0; i < set_size; i++)
	{
		LRU_block = &set[i];
		if (LRU_block->LRU < block->LRU){ //change this to only inc values LESS than block->LRU		
			LRU_block->LRU++;
		}		
		printf("--------------------\n");
		printf("BLOCK: %u\n", i);
		printf("B:Valid %u\n", LRU_block->valid);
		printf("B:Dirty %u\n", LRU_block->dirty);
		printf("B:Tag %lu\n", LRU_block->tag);
		printf("B:LRU %u\n", LRU_block->LRU);
	}
	
	block->tag = tag;
	block->valid = 1;
	block->LRU = 1;
	
	
	printf("------------------------------------------------------------\n");

    return(penalty);
}
