#include "BasicStackAllocator.h"

BasicStackAllocator::BasicStackAllocator()
{
	addBlock();
}

BasicStackAllocator::~BasicStackAllocator()
{
	while (!_blocks.empty()) {
		free(_blocks.top());
		_blocks.pop();
	}
}

char * BasicStackAllocator::allocate(size_t size)
{
	if (_position + size > _BLOCK_SIZE)
		addBlock();
	if (size > _BLOCK_SIZE)
		throw std::bad_alloc();
	char* answer = _blocks.top() + _position;
	_position += ((size - 1) / _ALIGN + 1) * _ALIGN; //allocates _ALIGN memory if size == 0 
	return answer;
}

void BasicStackAllocator::deallocate(char * const ptr, size_t size)
{
	//do nothing
}

size_t BasicStackAllocator::max_size()
{
	return _BLOCK_SIZE;
}

void BasicStackAllocator::addBlock()
{
	_blocks.push(reinterpret_cast<char*>(malloc(_BLOCK_SIZE)));
	_position = 0;
}
