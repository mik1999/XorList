#pragma once
#include <stack>
#include <memory>
#include <assert.h>

#include "BasicStackAllocator.cpp"


template <typename T>
class StackAllocator;

template <typename T1, typename T2>
bool operator==(const StackAllocator<T1>& lhs, const StackAllocator<T2>& rhs);


template <typename T>
class StackAllocator {
public:
	using value_type = T;
	using pointer = T * ;
	using const_pointer = const T *;
	using reference = T & ;
	using const_reference = const T &;

	template<class otherClass>
	struct rebind {
		using other = StackAllocator<otherClass>;
	};

	StackAllocator();
	StackAllocator(const StackAllocator &other);

	template <typename otherClass>
	StackAllocator(const StackAllocator <otherClass> &other);
	~StackAllocator();

	T * allocate(size_t size);
	void deallocate(T * const ptr, size_t size);

	size_t max_size() const;

private:
	template <typename T1>
	friend class StackAllocator;

	template <typename T1, typename T2>
	friend bool operator==(const StackAllocator<T1>& lhs, const StackAllocator<T2>& rhs);

	std::shared_ptr <BasicStackAllocator> _basicAlloc;

	static const size_t T_SIZE = sizeof(T);
};

template <typename T>
StackAllocator<T>::StackAllocator()
{
	_basicAlloc = std::make_shared<BasicStackAllocator>();
}

template <typename T>
StackAllocator<T>::StackAllocator(const StackAllocator &other) :
	_basicAlloc(other._basicAlloc)
{
	//initialize values
}

template <typename T>
template <typename otherClass>
StackAllocator<T>::StackAllocator(const StackAllocator <otherClass> &other) :
	_basicAlloc(other._basicAlloc)
{
	//initialize values
}

template <typename T>
StackAllocator<T>::~StackAllocator()
{
	//destroy values
}

template <typename T>
T * StackAllocator<T>::allocate(size_t size)
{
	return reinterpret_cast <T*> (_basicAlloc->allocate(size * T_SIZE));
}

template <typename T>
void StackAllocator<T>::deallocate(T * const ptr, size_t size)
{
	//do nothing
}

template<typename T>
size_t StackAllocator<T>::max_size() const
{
	return _basicAlloc->max_size() / T_SIZE;
}

template <typename T1, typename T2>
bool operator==(const StackAllocator<T1>& lhs, const StackAllocator<T2>& rhs)
{
	return lhs._basicAlloc.get() == rhs._basicAlloc.get();
}

template <typename T1, typename T2>
bool operator!=(const StackAllocator<T1>& lhs, const StackAllocator<T2>& rhs)
{
	return !(lhs == rhs);
}