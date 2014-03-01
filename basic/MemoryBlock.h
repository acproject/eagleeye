#ifndef _MEMORYBLOCK_H_
#define _MEMORYBLOCK_H_

#include "EagleeyeMacro.h"

#include <stdio.h>
#include <stdlib.h>
#include <algorithm>
namespace eagleeye
{
/**
 *	@brief memory block manage
 *	@note It manage memory dynamically. It don't support multi-threads
 */
class EAGLEEYE_API MemoryBlock
{
public:
	MemoryBlock();
	MemoryBlock(int size);
	MemoryBlock(const MemoryBlock& mem_block);
	~MemoryBlock();

	MemoryBlock& operator=(MemoryBlock mem_block);

	/**
	 *	@brief get memory block
	 */
	void* block();
	int blockSize();

private:
	void* m_block;
	int m_block_size;
	int* m_ref_count;
};
}

#endif
