#pragma once
#include <assert.h>
#include <iterator>
#include <iostream>

template <typename T>
struct errorType;

template <class T, class Allocator = std::allocator<T>>
class XorList {
public:
	explicit XorList(const Allocator& alloc = Allocator());
	XorList(const XorList&);
	XorList(XorList &&);
	~XorList();

	XorList& operator =(const XorList& other);
	XorList& operator =(XorList&& other);
	XorList(size_t count, const T& value = T(), const Allocator& alloc = Allocator());

	bool operator ==(const XorList &other) const;

	size_t size() const;
	bool empty() const;

	T back() const;
	T& back();
	T front() const;
	T& front();

	template <typename T1>
	void push_back(T1&&);

	template <typename T1>
	void push_front(T1&&);

	void pop_back();
	void pop_front();
private:
	struct _Node;
	typedef _Node* _pNode;
public:
	class iterator : public std::iterator <std::bidirectional_iterator_tag, T> {
	public:
		iterator() = default;
		iterator(const iterator & other) = default;
		iterator(_pNode prevNode, _pNode pNode);
		iterator& operator ++();
		iterator operator ++(int);
		iterator& operator --();
		iterator operator --(int);
		T& operator *();
		~iterator() = default;
		bool operator ==(const iterator& other) const;
		bool operator !=(const iterator& other) const;
	private:
		friend class XorList;
		_pNode _prevNode, _node;
	};

	template <typename T1>
	void insert_before(iterator, T1&&);

	template <typename T1>
	void insert_after(iterator, T1&&);

	void erase(iterator);

	iterator begin() const;
	iterator end() const;
private:
	void copy_elements(const XorList & other);
	void free_elements();

	typedef uintptr_t IntPtr;

	struct _Node {
	public:
		T _key;
		IntPtr _prevXorNext;
	};
	static void update(_pNode node, _pNode previous, _pNode next);
	static _pNode next(_pNode first, _pNode second = nullptr);
	static _pNode previous(_pNode first, _pNode second = nullptr);

	template <typename T1>
	void insert_between(_pNode first, _pNode second, T1&& value);

	_pNode create(T&&);
	_pNode create(const T&);
	void free(_pNode);

	using _XorListAllocator = typename Allocator::template rebind<_Node>::other;
	_XorListAllocator _xorListAlloc;

	_pNode _begin, _end;
	size_t _size;
};

template<class T, class Allocator>
XorList<T, Allocator>::XorList(const Allocator& alloc) : _size(0), _xorListAlloc(alloc)
{
	_begin = _end = nullptr;
}

template<class T, class Allocator>
XorList<T, Allocator>::XorList(const XorList<T, Allocator> & other) : _size(0)
{
	_xorListAlloc = std::allocator_traits<_XorListAllocator>::select_on_container_copy_construction(other._xorListAlloc);
	_begin = _end = nullptr;
	copy_elements(other);
}

template<class T, class Allocator>
XorList<T, Allocator>::XorList(XorList<T, Allocator> && other) :
	_size(other._size), _begin(other._begin), _end(other._end)
{
	_xorListAlloc = std::move<_XorListAllocator>(other._xorListAlloc);
	other._begin = other._end = nullptr;
	other._size = 0;
}

template<class T, class Allocator>
XorList<T, Allocator>::~XorList()
{
	free_elements();
}

template<class T, class Allocator>
XorList<T, Allocator> & XorList<T, Allocator>::operator=(const XorList<T, Allocator> & other)
{
	_xorListAlloc = std::allocator_traits<_XorListAllocator>::select_on_container_copy_construction(other._xorListAlloc);
	_size = 0;
	copy_elements(other);
	return *this;
}

template<class T, class Allocator>
XorList<T, Allocator> & XorList<T, Allocator>::operator=(XorList<T, Allocator> && other)
{
	_xorListAlloc = std::move<_XorListAllocator>(other._xorListAlloc);
	_begin = other._begin;
	_end = other._end;
	_size = other._size;
	other._begin = other._end = nullptr;
	other._size = 0;
	return *this;
}

template<class T, class Allocator>
XorList<T, Allocator>::XorList(size_t count, const T & value, const Allocator & alloc):_xorListAlloc(alloc)
{
	_begin = _end = nullptr;
	for (size_t i = 0; i < count; i++)
		push_back(value);
}

template<class T, class Allocator>
inline bool XorList<T, Allocator>::operator==(const XorList & other) const
{
	if (size() != other.size())
		return false;
	auto otherIt = other.begin();
	for (auto it = begin(); it != end(); ++it, ++otherIt)
		if (*it != *otherIt)
			return false;
	return true;
}

template<class T, class Allocator>
size_t XorList<T, Allocator>::size() const
{
	return _size;
}

template<class T, class Allocator>
bool XorList<T, Allocator>::empty() const
{
	return size() == 0;
}

template<class T, class Allocator>
T XorList<T, Allocator>::back() const
{
	return *(--end());
}

template<class T, class Allocator>
T & XorList<T, Allocator>::back()
{
	return *(--end());
}

template<class T, class Allocator>
T XorList<T, Allocator>::front() const
{
	return *begin();
}

template<class T, class Allocator>
T & XorList<T, Allocator>::front()
{
	return *begin();
}

template<class T, class Allocator>
template <typename T1>
void XorList<T, Allocator>::push_back(T1 && value)
{
	insert_before(end(), std::forward<T1>(value));
}

template<class T, class Allocator>
template<typename T1>
void XorList<T, Allocator>::push_front(T1 && value)
{
	insert_before(begin(), std::forward<T1>(value));
}

template<class T, class Allocator>
void XorList<T, Allocator>::pop_back()
{
	assert(!empty());
	erase(--end());
}

template<class T, class Allocator>
void XorList<T, Allocator>::pop_front()
{
	assert(!empty());
	erase(begin());
}

template<class T, class Allocator>
template<typename T1>
void XorList<T, Allocator>::insert_before(iterator it, T1 && value)
{
	insert_between(it._prevNode, it._node, std::forward<T1>(value));
}

template<class T, class Allocator>
template<typename T1>
void XorList<T, Allocator>::insert_after(iterator it, T1 && value)
{
	assert(it._node != nullptr); //it != end()
	insert_between(it._node, next(it._prevNode, it._node), std::forward<T1>(value));
}

template<class T, class Allocator>
void XorList<T, Allocator>::erase(iterator it)
{
	assert(it._node != nullptr); //it != end()
	_size--;
	_pNode lNode = it._prevNode, delNode = it._node, rNode = next(lNode, delNode);
	if (lNode != nullptr)
		update(lNode, previous(lNode, delNode), rNode);
	else
		_begin = rNode;
	if (rNode != nullptr)
		update(rNode, lNode, next(delNode, rNode));
	else
		_end = lNode;
	free(delNode);
}

template<class T, class Allocator>
typename XorList<T, Allocator>::iterator XorList<T, Allocator>::begin() const
{
	return iterator(nullptr, _begin);
}

template<class T, class Allocator>
typename XorList<T, Allocator>::iterator XorList<T, Allocator>::end() const
{
	return iterator(_end, nullptr);
}

template<class T, class Allocator>
void XorList<T, Allocator>::copy_elements(const XorList & other)
{
	for (iterator it = other.begin(); it != other.end(); it++)
		push_back(*it);
}

template<class T, class Allocator>
void XorList<T, Allocator>::free_elements()
{
	while (!empty())
		pop_front();
}

template<class T, class Allocator>
typename XorList<T, Allocator>::_pNode XorList<T, Allocator>::next(_pNode first, _pNode second)
{
	return _pNode(IntPtr(first) ^ second->_prevXorNext);
}

template<class T, class Allocator>
typename XorList<T, Allocator>::_pNode XorList<T, Allocator>::previous(_pNode first, _pNode second)
{
	return _pNode(first->_prevXorNext ^ IntPtr(second));
}

template<class T, class Allocator>
template<typename T1>
void XorList<T, Allocator>::insert_between(_pNode first, _pNode second, T1 && value)
{
	_pNode newNode = create(std::forward<T1>(value));
	_size++;
	if (first != nullptr)
		update(first, previous(first, second), newNode);
	else
		_begin = newNode;
	if (second != nullptr)
		update(second, newNode, next(first, second));
	else
		_end = newNode;
	update(newNode, first, second);
}

template<class T, class Allocator>
typename XorList<T, Allocator>::_pNode XorList<T, Allocator>::create(T && value)
{
	auto ptr = new(_xorListAlloc.allocate(1)) _Node;
	ptr->_key = std::move(std::forward<T>(value));
	return ptr;
}

template<class T, class Allocator>
typename XorList<T, Allocator>::_pNode XorList<T, Allocator>::create(const T & value)
{
	auto ptr = new(_xorListAlloc.allocate(1)) _Node;
	ptr->_key = value;
	return ptr;
}

template<class T, class Allocator>
void XorList<T, Allocator>::free(_pNode pnode)
{
	_xorListAlloc.deallocate(pnode, 1);
}

template<class T, class Allocator>
XorList<T, Allocator>::iterator::iterator(_pNode prevNode, _pNode pNode) : _prevNode(prevNode), _node(pNode) {
	//initialize values
}

template<class T, class Allocator>
typename XorList<T, Allocator>::iterator& XorList<T, Allocator>::iterator::operator++()
{
	assert(_node != nullptr); //it != end()
	_prevNode = next(_prevNode, _node);
	std::swap(_prevNode, _node);
	return *this;
}

template<class T, class Allocator>
typename XorList<T, Allocator>::iterator XorList<T, Allocator>::iterator::operator++(int)
{
	assert(_node != nullptr); //it != end()
	iterator old = *this;
	_prevNode = next(_prevNode, _node);
	std::swap(_prevNode, _node);
	return old;
}

template<class T, class Allocator>
typename XorList<T, Allocator>::iterator& XorList<T, Allocator>::iterator::operator--()
{
	assert(_prevNode != nullptr); //it != begin()
	_node = previous(_prevNode, _node);
	std::swap(_prevNode, _node);
	return *this;
}

template<class T, class Allocator>
typename XorList<T, Allocator>::iterator XorList<T, Allocator>::iterator::operator--(int)
{
	assert(_prevNode != nullptr); //it != begin()
	iterator old = *this;
	_node = previous(_prevNode, _node);
	std::swap(_prevNode, _node);
	return old;
}

template<class T, class Allocator>
T& XorList<T, Allocator>::iterator::operator*()
{
	assert(_node != nullptr); //it != end()
	return _node->_key;
}

template<class T, class Allocator>
inline bool XorList<T, Allocator>::iterator::operator==(const iterator & other) const
{
	return _node == other._node && _prevNode == other._prevNode;
}

template<class T, class Allocator>
inline bool XorList<T, Allocator>::iterator::operator!=(const iterator & other) const
{
	return !operator == (other);
}

template<class T, class Allocator>
void XorList<T, Allocator>::update(_pNode node, _pNode previous, _pNode next)
{
	node->_prevXorNext = IntPtr(previous) ^ IntPtr(next);
}