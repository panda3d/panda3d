
#include "stdafx.h"

#include <iostream>
#include <map>
#include <cassert>
using namespace std;

#include "uniqueIdAllocator.h"


int _tmain(int argc, _TCHAR* argv[]) {
	cout <<"UniqueIdAllocator Test"<<endl;
	UniqueIdAllocator b=UniqueIdAllocator(2, 9);
	b.print_to(cout, true);
	b.allocate(); b.print_to(cout, true);
	b.allocate(); b.print_to(cout, true);
	b.allocate(); b.print_to(cout, true);
	b.allocate(); b.print_to(cout, true);
	b.allocate(); b.print_to(cout, true);
	b.allocate(); b.print_to(cout, true);
	b.free(2); b.print_to(cout, true);
	b.free(3); b.print_to(cout, true);
	b.free(4); b.print_to(cout, true);
	b.free(5); b.print_to(cout, true);
	b.allocate(); b.print_to(cout, true);
	b.allocate(); b.print_to(cout, true);
	b.allocate(); b.print_to(cout, true);
	b.free(3); b.print_to(cout, true);
	b.free(2); b.print_to(cout, true);

	b.allocate(); b.print_to(cout, true);
	b.allocate(); b.print_to(cout, true);
	b.allocate(); b.print_to(cout, true);
	b.allocate(); b.print_to(cout, true);
	b.allocate(); b.print_to(cout, true);
	b.allocate(); b.print_to(cout, true);
	b.allocate(); b.print_to(cout, true);

	b.free(4); b.print_to(cout, true);
	b.free(3); b.print_to(cout, true);

	b.allocate(); b.print_to(cout, true);

	return 0;
}
