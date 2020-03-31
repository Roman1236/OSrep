#include <iostream>
#include "string.h"
#include "Allocator.h"
using namespace std;

int main()
{
	auto allocator = new Allocator();

	char ch1 = '1';
	char* ch2 = &ch1;	

	auto mem1 = allocator->mem_alloc(sizeof(&ch2));
	cout << mem1<<endl;
	auto mem2 = allocator->mem_realloc(ch2, sizeof(&ch2));
	cout << mem2 << endl;
			
	return 0;
}