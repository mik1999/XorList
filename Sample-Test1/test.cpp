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
#include "../XorList/ListOperation.h"


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



template <typename T, class List1, class List2>
void testAllocators(
	List1 &list1, List2 &list2, const std::list<ListOperation<T> > &opList) {
	ListOperation<T>::discardStatic();
	for (auto op : opList)
		doOperationAndCheck<T, List1, List2>(list1, list2, op);
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
		10000, 30000, 100000, 300000, 1000000, 3000000, 10000000};
	for (auto num : cntOfOpsToTestOn)
		compareWorkingTime(num, result);
	result.close();
}

int main(int argc, char **argv) {
	::testing::InitGoogleTest(&argc, argv);
	return RUN_ALL_TESTS();
}