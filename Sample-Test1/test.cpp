#include "pch.h"
#include <iostream>
#include <algorithm>
#include <random>
#include <list>
#include <memory>
#include <fstream>
#include <utility>

#include "../XorList/StackAllocator.h"
#include "../XorList/XorList.h"

std::random_device my_rand;

//join two lists
template <typename T>
std::list<T>& join(std::list<T> &list1, std::list<T> &&list2) {
	while (!list2.empty()) {
		list1.push_back(list2.front());
		list2.pop_front();
	}
	return list1;
}

TEST(TestStackAllocator, SimpleTest) {
	const int ArraySize = 100;

	StackAllocator <int> alloc;
	int* array = alloc.allocate(ArraySize),
		*sum = alloc.allocate(1), *ans = alloc.allocate(1);
	*ans = ArraySize * (ArraySize + 1) / 2;
	*sum = 0;
	for (size_t i = 0; i < ArraySize; i++)
		array[i] = i + 1;
	std::for_each(array, array + ArraySize, [sum](int &n) {*sum += n; });
	ASSERT_EQ(*ans, *sum);
}


template <class Allocator>
class RandomSizedAllocation {
public:
	RandomSizedAllocation() = delete;
	template <class Allocator>
	RandomSizedAllocation(const Allocator& alloc) {
		_alloc = alloc;
		_size = my_rand() % alloc.max_size();
		_pointer = _alloc.allocate(_size);
	}
	~RandomSizedAllocation() {
		_alloc.deallocate(_pointer, _size);
	}
private:
	size_t _size;
	typename std::allocator_traits<Allocator>::pointer _pointer;
	Allocator _alloc;
};

TEST(TestStackAllocator, CopyIntTest) {

	StackAllocator <int> intAl;
	StackAllocator <int> anotherIntAl;

	StackAllocator <int> intAl1 = intAl;
	StackAllocator <int> intAl2 = intAl1;

	StackAllocator <int> otherAl1;
	StackAllocator <int> otherAl2;
	StackAllocator <int> otherAl3;

	otherAl1 = intAl1;
	otherAl2 = otherAl1;
	otherAl3 = intAl;

	RandomSizedAllocation<StackAllocator <int> > RG1(intAl);
	RandomSizedAllocation<StackAllocator <int> > RG2(intAl1);
	RandomSizedAllocation<StackAllocator <int> > RG3(intAl2);
	RandomSizedAllocation<StackAllocator <int> > RG4(otherAl1);
	RandomSizedAllocation<StackAllocator <int> > RG5(otherAl2);
	RandomSizedAllocation<StackAllocator <int> > RG6(otherAl3);

	ASSERT_TRUE(intAl == intAl1);
	ASSERT_TRUE(intAl1 == intAl2);
	ASSERT_TRUE(intAl == intAl2);
	ASSERT_TRUE(intAl == otherAl3);
	ASSERT_TRUE(intAl == otherAl2);
	ASSERT_TRUE(otherAl2 == otherAl3);
	ASSERT_TRUE(intAl2 == otherAl2);
	ASSERT_TRUE(intAl != anotherIntAl);
}


TEST(TestStackAllocator, CopyDiffTest) {
	StackAllocator <int> intAl;
	StackAllocator <bool> boolAl;

	StackAllocator <double> doubleAl = intAl;
	StackAllocator <std::string>  strAl = doubleAl;
	StackAllocator <StackAllocator <size_t> > alAl = intAl;

	ASSERT_TRUE(intAl == doubleAl);
	ASSERT_TRUE(intAl == alAl);
	ASSERT_TRUE(doubleAl == strAl);
	ASSERT_TRUE(doubleAl == alAl);
	ASSERT_TRUE(intAl != boolAl);
}


template <typename T>
class ListOperation {
public:
	enum Kind {
		LKsize,
		LKpush_back,
		LKpush_front,
		LKpop_back,
		LKpop_front,
		LKback,
		LKfront,
		LKget_by_iterator_from_begin,
		LKget_by_iterator_from_end,
	};
	ListOperation() = delete;
	ListOperation(Kind kind, size_t size_t_value, const T& T_value);

	static size_t last_size_t_answer();
	static T last_T_answer();

	template <class List>
	void operator()(List& list);
private:
	Kind _kind;
	size_t _size_t_value;
	T _T_value;

	static size_t _last_size_t_answer;
	static T _last_T_answer;
	static void discardStatic();

	template <class List>
	static typename List::iterator _get_iterator_from_begin(List &list, size_t pos);
	//including begin

	template <class List>
	static typename List::iterator _get_iterator_from_end(List &list, size_t pos);
	//excluding end

};

template<typename T>
size_t ListOperation<T>::_last_size_t_answer;

template<typename T>
T ListOperation<T>::_last_T_answer;


template<typename T>
template<class List>
typename List::iterator ListOperation<T>::_get_iterator_from_begin(List &list, size_t pos)
{
	typename List::iterator it = list.begin();
	for (size_t i = 0; i < pos; ++i)
		it++;
	return it;
}

template<typename T>
template<class List>
typename List::iterator ListOperation<T>::_get_iterator_from_end(List &list, size_t pos)
{
	typename List::iterator it = list.end();
	for (size_t i = 0; i <= pos; ++i)
		it--;
	return it;
}

template<typename T>
template<class List>
void ListOperation<T>::operator()(List & list)
{
	switch (_kind)
	{
	case LKsize:
		_last_size_t_answer = list.size();
		break;
	case LKpush_back:
		list.push_back(_T_value);
		break;
	case LKpush_front:
		list.push_front(_T_value);
		break;
	case LKpop_back:
		list.pop_back();
		break;
	case LKpop_front:
		list.pop_front();
		break;
	case LKback:
		_last_T_answer = list.back();
		break;
	case LKget_by_iterator_from_begin:
		_last_T_answer = *_get_iterator_from_begin<List>(list, _size_t_value);
		break;
	case LKget_by_iterator_from_end:
		_last_T_answer = *_get_iterator_from_end<List>(list, _size_t_value);
		break;
	default:
		break;
	}
}

template <typename T, T generate()>
ListOperation <T> generate_random_insertion_operation() {
	int id = my_rand() % 2;
	switch (id)
	{
	case 0:
		return ListOperation<T>(ListOperation<T>::Kind::LKpush_back, 0, generate());
	case 1:
		return ListOperation<T>(ListOperation<T>::Kind::LKpush_front, 0, generate());
	default:
		break;
	}
}

template <typename T>
ListOperation <T> generate_random_erase_operation() {
	int id = my_rand() % 2;
	switch (id)
	{
	case 0:
		return ListOperation<T>(ListOperation<T>::Kind::LKpop_back, 0, T());
	case 1:
		return ListOperation<T>(ListOperation<T>::Kind::LKpop_front, 0, T());
	}
}

template <typename T>
ListOperation <T> generate_random_access_operation(size_t size) {
	int id = my_rand() % 5;
	switch (id)
	{
	case 0:
		return ListOperation<T>(ListOperation<T>::Kind::LKsize, 0, 0);
	case 1:
		return ListOperation<T>(ListOperation<T>::Kind::LKback, 0, 0);
	case 2:
		return ListOperation<T>(ListOperation<T>::Kind::LKfront, 0, 0);
	case 3:
		return ListOperation<T>(
			ListOperation<T>::Kind::LKget_by_iterator_from_begin, 0, 0);
	case 4:
		return ListOperation<T>(
			ListOperation<T>::Kind::LKget_by_iterator_from_end, 0, 0);
	default:
		break;
	}
}

enum GenerationFlag {
	GFcreate,
	GFfree,
	GFaccess,
	GFno_specification
};

std::pair <int, int> generationFlagBourders(GenerationFlag flag) {
	switch (flag)
	{
	case GFcreate:
		return std::make_pair<int>(4, 5);
	case GFfree:
		return std::make_pair<int>(1, 5);
	case GFaccess:
		return std::make_pair<int>(1, 2);
	case GFno_specification:
		return std::make_pair<int>(2, 4);
	}
}

template <typename T, T generate() = T()>
std::list <ListOperation<T> > generate_operation_list(
	size_t number, size_t& size, GenerationFlag flag = GFno_specification) {
	std::list<ListOperation<T> > answer;
	for (size_t i = 0; i < number; i++) {
		if (size == 0) {
			size++;
			answer.push_back(generate_random_insertion_operation<T, generate>());
			continue;
		}
		int op_id = my_rand() % 6;
		std::pair <int, int> bourders = generationFlagBourders(flag);
		if (op_id < bourders.first) {
			answer.push_back(generate_random_insertion_operation<T, generate>());
			size++;
		}
		else if (op_id < bourders.second) {
			answer.push_back(generate_random_erase_operation<T>());
			size--;
		}
		else
			answer.push_back(generate_random_access_operation<T>(size));
	}
	return answer;
}

template<typename T>
ListOperation<T>::ListOperation(Kind kind, size_t size_t_value, const T & T_value) :
	_kind(kind), _size_t_value(size_t_value), _T_value(T_value)
{
	//initialize values;
}

template<typename T>
size_t ListOperation<T>::last_size_t_answer()
{
	return _last_size_t_answer;
}

template<typename T>
T ListOperation<T>::last_T_answer()
{
	return _last_T_answer;
}

template<typename T>
void ListOperation<T>::discardStatic()
{
	_last_size_t_answer = 0;
	_last_T_answer = T();
}

template <typename T, class List1, class List2>
void doOperationAndCheck(
	List1 &list1, List2 &list2, ListOperation<T> &op) {
	size_t size_t_answer1, size_t_answer2;
	T T_answer1, T_answer2;
	op(list1);
	size_t_answer1 = ListOperation<T>::last_size_t_answer();
	T_answer1 = ListOperation<T>::last_T_answer();
	op(list2);
	size_t_answer2 = ListOperation<T>::last_size_t_answer();
	T_answer2 = ListOperation<T>::last_T_answer();
	ASSERT_EQ(size_t_answer1, size_t_answer2);
	ASSERT_EQ(T_answer1, T_answer2);
}

template <typename T, class List1, class List2>
void testAllocators(
	List1 &list1, List2 &list2, const std::list<ListOperation<T> > &opList) {
	ListOperation<T>::discardStatic();
	for (auto op : opList)
		doOperationAndCheck<T, List1, List2>(list1, list2, op);
}

template <typename T, class List1>
void doOperations(List1 &list, const std::list<ListOperation<T> > &opList) {
	for (auto op : opList)
		op(list);
}


template <typename T, T generate() = T()>
std::list<ListOperation<T> > generateRandomLeapOperations(size_t num) {
	std::list<ListOperation<T> > answer;
	size_t size = 0;
	answer = join(answer, generate_operation_list<T, generate>(num / 4, size, GFcreate));
	answer = join(answer, generate_operation_list<T, generate>(num / 4, size, GFno_specification));
	answer = join(answer, generate_operation_list<T, generate>(num / 4, size, GFfree));
	answer = join(answer, generate_operation_list<T, generate>(num - 3 * (num / 4), size, GFaccess));
	return answer;

}

template <typename T, T generate() = T()>
std::list<ListOperation<T> > generateRandomStaticOperations(size_t num) {
	std::list<ListOperation<T> > answer;
	size_t size = 0;
	answer = join(answer, generate_operation_list<T, generate>(num / 10, size, GFcreate));
	answer = join(answer, generate_operation_list<T, generate>(num - (num / 10), size, GFno_specification));
	return answer;
}

template <typename T, T generate() = T()>
std::list<ListOperation<T> > generateRandomLadderOperations(size_t num) {
	std::list<ListOperation<T> > answer;
	const size_t LEAP_OPERATION_COUNT = 3000;
	auto kind = GFcreate;
	size_t size = 0;
	while (true) {
		answer = join(answer, generate_operation_list<T, generate>(
			std::min(num, LEAP_OPERATION_COUNT), size, kind));
		if (kind == GFcreate)
			kind = GFfree;
		else
			kind = GFcreate;
		if (num <= LEAP_OPERATION_COUNT)
			break;
		num -= LEAP_OPERATION_COUNT;
	}
	return answer;
}
template <typename T>
bool operator ==(const std::vector <T> &vec, const XorList<T> list) {
	std::vector <T> buf(vec.size());
	std::copy(list.begin(), list.end(), buf.begin());
	return vec == buf;
}

TEST(TestStackAllocator, STDlist) {
	std::list<int, StackAllocator<int> > STDlist;
	auto ops = generateRandomLeapOperations<int, rand>(10000);
	doOperations(STDlist, ops);
	ASSERT_TRUE(true);
}

TEST(TestXorList, SimpleTest) {
	XorList <int> list;
	list.push_back(4);
	list.push_front(2);
	list.insert_after(list.begin(), 3);
	list.insert_before(list.begin(), 1);
	std::vector <int> ans{ 1, 2, 3, 4 }, vec(4);
	std::copy(list.begin(), list.end(), vec.begin());
	ASSERT_TRUE(vec == ans);
}

TEST(TestXorList, CopyTest) {
	XorList<int> intList1;
	doOperations(intList1, generateRandomStaticOperations<int, rand>(10));
	XorList<int> intList2 = intList1;
	XorList<int> intList3 = intList2;
	XorList<int> intList4;
	intList4 = intList3;
	ASSERT_TRUE(intList1 == intList3);
	ASSERT_TRUE(intList2 == intList4);
	ASSERT_TRUE(intList1 == intList4);
	intList4.push_back(0);
	ASSERT_FALSE(intList3 == intList4);
}

void testWithSTDList(std::list<ListOperation<int> > ops) {
	std::list<int> STDList;
	XorList<int> xorList;
	for (auto op : ops)
		doOperationAndCheck(STDList, xorList, op);
}

TEST(TestXorList, CompareWithSTDList1) {
	testWithSTDList(generateRandomStaticOperations<int, rand>(100));
}

TEST(TestXorList, CompareWithSTDList2) {
	testWithSTDList(generateRandomStaticOperations<int, rand>(3000));
}

TEST(TestXorList, CompareWithSTDList3) {
	testWithSTDList(generateRandomStaticOperations<int, rand>(10000));
}

TEST(TestXorList, CompareWithSTDListLeap1) {
	testWithSTDList(generateRandomLeapOperations<int, rand>(100));
}

TEST(TestXorList, CompareWithSTDListLeap2) {
	testWithSTDList(generateRandomLeapOperations<int, rand>(3000));
}

TEST(TestXorList, CompareWithSTDListLeap3) {
	testWithSTDList(generateRandomLeapOperations<int, rand>(10000));
}

TEST(TestXorList, CompareWithSTDListLadder1) {
	testWithSTDList(generateRandomLadderOperations<int, rand>(100));
}

TEST(TestXorList, CompareWithSTDListLadder2) {
	testWithSTDList(generateRandomLadderOperations<int, rand>(3000));
}

TEST(TestXorList, CompareWithSTDListLadder3) {
	testWithSTDList(generateRandomLadderOperations<int, rand>(10000));
}

template <typename T>
void printOnWidth(std::ofstream &out, const T& data) {
	static const size_t WIDTH = 30;
	out << std::setw(WIDTH) << data;
}

template <class List>
double workingTime(List &list, std::list<ListOperation<int> > ops) {
	clock_t begTime = clock();
	for (auto op : ops)
		op(list);
	clock_t endTime = clock();
	return double(endTime - begTime) / CLOCKS_PER_SEC;
}

void compareWorkingTime(size_t numOfOps, std::ofstream &result) {
	std::list<ListOperation<int> > ops = generateRandomStaticOperations<int, rand>(numOfOps);
	std::list<int, std::allocator<int> > STDlist1;
	std::list<int, StackAllocator<int> > STDlist2;
	XorList<int, std::allocator<int> > xorList1;
	XorList<int, StackAllocator<int> > xorList2;
	printOnWidth(result, numOfOps);
	printOnWidth(result, workingTime(STDlist1, ops));
	printOnWidth(result, workingTime(STDlist2, ops));
	printOnWidth(result, workingTime(xorList1, ops));
	printOnWidth(result, workingTime(xorList2, ops));
	result << std::endl;
}

TEST(TestXorList, CompareWorkingTimeDependingOnAllocator) {
	std::ofstream result("Working_time_compare_result.txt");

	result.fill(' ');
	printOnWidth(result, "Amont of operations");
	printOnWidth(result, "std::list<std::allocator>");
	printOnWidth(result, "std::list<StackAlloc>");
	printOnWidth(result, "XorList<std::allocator>");
	printOnWidth(result, "XorList<StackAlloc>");
	result <<  std::endl << std::fixed << std::setprecision(3);
	const std::vector<size_t> cntOfOpsToTestOn{
		1000, 3000, 10000, 30000, 100000, 300000, 1000000};
	for (auto num : cntOfOpsToTestOn)
		compareWorkingTime(num, result);
	result.close();
}

int main(int argc, char **argv) {
	::testing::InitGoogleTest(&argc, argv);
	return RUN_ALL_TESTS();
}