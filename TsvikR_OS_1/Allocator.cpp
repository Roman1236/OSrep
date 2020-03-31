#include "Allocator.h"
#include "Windows.h"

Allocator::Allocator()
{
	used_blocks = new std::list<mem_block*>();
	free_blocks = new std::list<mem_block*>();
	last_block_ = nullptr;
}

Allocator::mem_block* Allocator::join_blocks(mem_block* left, mem_block* right)
{
	left->size += right->size;
	left->next = right->next;
	return left;
}

size_t Allocator::align_size(size_t size)
{
	return (size + sizeof(intptr_t) - 1) & ~(sizeof(intptr_t) - 1);
}

void* Allocator::mem_alloc(size_t size)
{
	size = align_size(size);
	
	auto block = find_first_block(size);

	if (block == nullptr)
	{
		block = alloc_heap_mem_block(size);

		if (block == nullptr)
		{
			return nullptr;
		}
	}

	split_block(block, size);
	
	return block->ptr;
}

void* Allocator::mem_realloc(void* addr, size_t size)
{
	mem_free(addr);
	return mem_alloc(size);
}

void Allocator::mem_free(void* addr)
{
	const auto ptr = reinterpret_cast<uint8_t*>(addr);

	for (auto block : *used_blocks)
	{
		if (block->ptr != ptr) continue;

		used_blocks->remove(block);
		free_blocks->push_back(block);
		join_block(block);
		return;
	}
}



Allocator::mem_block* Allocator::find_first_block(const size_t size)
{
	for (auto block : *free_blocks)
	{
		if (block->size < size) continue;
		return block;
	}

	return nullptr;
}

Allocator::mem_block* Allocator::alloc_heap_mem_block(size_t size)
{
	const auto mem = HeapAlloc(GetProcessHeap(), HEAP_ZERO_MEMORY, size);

	if (mem == nullptr)
	{
		return nullptr;
	}
	
	const auto block = new mem_block();
	block->next = nullptr;
	block->prev = nullptr;
	block->ptr = static_cast<uint8_t*>(mem);
	block->size = size;
	
	free_blocks->push_back(block);

	if (last_block_ != nullptr)
	{
		last_block_->next = block;
		block->prev = last_block_;
	}

	last_block_ = block;
	
	return block;
}

void Allocator::split_block(mem_block *block, const size_t size)
{
	free_blocks->remove(block);
	used_blocks->push_back(block);
	
	if (block->size == size) return;

	const auto size_diff = block->size - size;

	const auto splitted_block = new mem_block();
	splitted_block->ptr = block->ptr + size;
	splitted_block->size = size_diff;
	splitted_block->prev = block;
	splitted_block->next = block->next;
	
	block->next = splitted_block;
	block->size = size;

	free_blocks->push_back(splitted_block);
}

void Allocator::join_block(mem_block *block)
{
	if (block-> prev != nullptr && (block->prev + block->prev->size == block) && contains_block(free_blocks, block->prev))
	{
		block = join_blocks(block->prev, block);
	}

	if (block->next != nullptr && (block + block->size == block->next) && contains_block(free_blocks, block->next))
	{
		block = join_blocks(block->next, block);
	}
}



bool Allocator::contains_block(const std::list<mem_block*>* list, mem_block* block)
{
	for (auto list_block : *list)
	{
		if (block == nullptr) continue;
		if (list_block->ptr == block->ptr) return true;
	}

	return false;
}
