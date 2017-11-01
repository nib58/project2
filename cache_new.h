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
	unsigned int offset_size = log2(cp->blocksize);//correct
	unsigned int index_size = log2(cp->nsets);//correct
	unsigned int tag_size = 32 - (offset_size + index_size);//correct
	
	unsigned int offset_lshift = tag_size + index_size;//changed
	unsigned int offset_rshift = tag_size + index_size;//changed
	unsigned int index_lshift = tag_size;//changed
	unsigned int index_rshift = tag_size + offset_size;//correct
	unsigned int tag_rshift = offset_size + index_size;//correct
	
	unsigned int offset = address << offset_lshift;//correct
	offset = offset >> offset_rshift;//correct
	unsigned int index = address << index_lshift;//correct
	index = index >> index_rshift;//correct
	unsigned int tag = address >> tag_rshift;//correct
	
	// Check Blocks
	int i;
	int hit = 0;
	struct cache_blk_t * block;
	struct cache_blk_t * set = cp->blocks[index];
	struct cache_blk_t LRU[index_size];
	
	for(i = 0; i < (cp->assoc); i++)
	{
		block = &set[i];
		if (block->valid == 1)
		{
			if (block->tag == tag)//good
			{	// HIT
				hit = 1;
				break;
			}
		}
		else
		{	// MISS
			break;
		}
	}
	
	/*
	for(i = 0; i < (cp->assoc); i++)
	{
		block = set[i];
		
		if (access_type == 0)
		{	// READ
			// Potential Read Hit
			if (block->valid == 1)
			{
				if (block->tag == tag)
				{	// HIT
					hit = 1;
					break;
				}
			}
			else
			{	// MISS
				break;
			}
		}
		else
		{	// WRITE
			// Potential Write Hit
			if (block->valid == 1)
			{
				if (block->tag == tag)
				{	// HIT
					hit = 1;
					break;
				}
			}
			else
			{	// MISS
				break;
			}
		}
	}
	*/
	
	if (hit == 1)//good
	{	// HIT
		// LRU Update
		penalty = 0;
	}
	else
	{	// MISS
		// LRU Get
		// block = LRU;
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
			block->valid = 1;
		}
	}

	if (access_type == 0)
	{	// WRITE
		// Write Always Sets Dirty Bit
		block->dirty = 1;
		block->valid = 1;
	}
	
	/*
	if (hit == 1):
	{	// HIT		
		if (access_type == 0)
		{	// READ
			penalty = 0;
			// LRU Add
			continue;
		}
		else
		{	// WRITE
			
			
			
			// Write Always Sets Block To Dirty
			block->dirty = 1;
		}
	}
	else
	{	// MISS
		
		if (access_type == 0)
		{	// READ
			if (i == ((cp->assoc) - 1))
			{	// LRU Get
				// block = LRU;
				if (block->dirty == 1)
				{
					penalty = penalty * 2;
				}
				else
				{
					block->tag = tag;
					block->valid = 1;
				}
			}
			else
			{	// Write To First Invalid Block
				block->tag = tag;
				block->valid = 1;
			}
		}
		else
		{	// WRITE
			if (i == ((cp->assoc) - 1))
			{	// LRU Get
				// block = LRU;
				if (block->dirty == 1)
				{
					penalty = penalty * 2;
				}
				else
				{
					
				}
				
			}
			else
			{	// Write To First Invalid Block
				block->tag = tag;
				block->valid = 1;
			}
			
			// Write Always Sets Block To Dirty
			block->dirty = 1;
		}
	}
	*/

    return(penalty);
}