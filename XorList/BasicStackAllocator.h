#pragma once

#include <stack>
#include <algorithm>

class BasicStackAllocator {
public:
	BasicStackAllocator();
	~BasicStackAllocator();

	char * allocate(size_t size);
	void deallocate(char * const ptr, size_t size);
	size_t max_size();
private:
	static const size_t _ALIGN = alignof(std::max_align_t);
	static const size_t _BLOCK_SIZE = (1 << 17) * _ALIGN;

	std::stack <char*> _blocks;
	size_t _position;

	void addBlock();
};
