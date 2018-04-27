#include <iostream>
#include <memory>
#include <list>

#include "BasicStackAllocator.h"

using namespace std;

int main() {
	list <int> lis;
	lis.push_back(int());
	list <int>::iterator it = lis.begin();
	it++;
	system("pause");
	return 0;
}