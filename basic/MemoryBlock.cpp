#include "MemoryBlock.h"

namespace eagleeye
{
MemoryBlock::MemoryBlock()
{
	m_block = NULL;
	m_ref_count = new int(1);
	m_block_size = 0;
}

MemoryBlock::MemoryBlock(int size)
{
	m_block = malloc(size);
	m_ref_count = new int(1);
	m_block_size = size;
}

MemoryBlock::MemoryBlock(const MemoryBlock& mem_block)
	:m_block(mem_block.m_block),
	m_block_size(mem_block.m_block_size),
	m_ref_count(mem_block.m_ref_count)
{
	++*m_ref_count;	
}

MemoryBlock::~MemoryBlock()
{
	if (m_ref_count)
	{
		if (--*m_ref_count == 0)
		{
			if (m_block)
				free(m_block);
			delete m_ref_count;
		}
	}
}

MemoryBlock& MemoryBlock::operator=(MemoryBlock mem_block)
{
	void* temp_block;
	int* temp_ref_count;
	int temp_block_size;

	temp_block = mem_block.m_block;
	temp_ref_count = mem_block.m_ref_count;
	temp_block_size = mem_block.m_block_size;

	mem_block.m_block = m_block;
	mem_block.m_ref_count = m_ref_count;
	mem_block.m_block_size = m_block_size;

	m_block = temp_block;
	m_ref_count = temp_ref_count;
	m_block_size = temp_block_size;

	return *this;
}

void* MemoryBlock::block()
{
	return m_block;
}
int MemoryBlock::blockSize()
{
	return m_block_size;
}
}