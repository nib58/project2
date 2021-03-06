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
	if (x == 1)
	{
		return 1;
	}
	
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
	
	// Handle Single Set
	if (cp->nsets == 1)
	{
		index = 0;
	}

    // Check Blocks
    int i;

    int hit = 0;
    int set_size = cp->assoc;
    int block_index = set_size;
    int LRU_value = 0;
    int LRU_comp = -1;

    struct cache_blk_t * block;
    struct cache_blk_t * set_block;
    struct cache_blk_t * LRU_block;
    struct cache_blk_t * set = cp->blocks[index];

    for(i = 0; i < set_size; i++)
    {
        set_block = &set[i];
        if (set_block->valid == 1)
        {
            if (set_block->tag == tag)
            {   // HIT
                block_index = i;
                hit = 1;
            }
        }
        else
        {   // MISS
            set_block->LRU = set_size;
        }
    }
	
    // Reset Block To Hit If Possible
    if (i == set_size)
    {
        block = &set[block_index];
    }

    if (hit == 1)
    {   // HIT
        penalty = 0;
    }
    else
    {   // MISS
        // LRU Get
        // Block Should Be LRU Block;
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
        {   // Write Back
            penalty = penalty * 2;
        }

        if (access_type == 0)
        {   // READ
            // Read Resets Dirty Bit On Miss
            block->dirty = 0;
        }
    }

    if (access_type == 1)
    {   // WRITE
        // Write Always Sets Dirty Bit
        block->dirty = 1;
    }

    // LRU Update
	LRU_comp = block->LRU;
    for(i = 0; i < set_size; i++)
    {	
		set_block = &set[i];
        if ((i != block_index) && (set_block->LRU <= LRU_comp))
        {   // Only Increment Values Lower Than Block's LRU
            set_block->LRU++;
        }
    }
	
	// Update Block Values
	block->tag = tag;
    block->valid = 1;
	block->LRU = 0;
    return(penalty);
}
