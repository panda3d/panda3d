

#include "stdafx.h"

#include <iostream>
#include <map>
#include <cassert>
using namespace std;

#include "uniqueIdAllocator.h"


int _tmain(int argc, _TCHAR* argv[]) {
	cout <<"UniqueIdAllocator Test"<<endl;
	UniqueIdAllocator b=UniqueIdAllocator(2, 9);
	b.printTo(cout);
	b.allocate(); b.printTo(cout);
	b.allocate(); b.printTo(cout);
	b.allocate(); b.printTo(cout);
	b.allocate(); b.printTo(cout);
	b.allocate(); b.printTo(cout);
	b.allocate(); b.printTo(cout);
	b.free(2); b.printTo(cout);
	b.free(3); b.printTo(cout);
	b.free(4); b.printTo(cout);
	b.free(5); b.printTo(cout);
	b.allocate(); b.printTo(cout);
	b.allocate(); b.printTo(cout);
	b.allocate(); b.printTo(cout);
	b.free(3); b.printTo(cout);
	b.free(2); b.printTo(cout);

	b.allocate(); b.printTo(cout);
	b.allocate(); b.printTo(cout);
	b.allocate(); b.printTo(cout);
	b.allocate(); b.printTo(cout);
	b.allocate(); b.printTo(cout);
	b.allocate(); b.printTo(cout);
	b.allocate(); b.printTo(cout);

	b.free(4); b.printTo(cout);
	b.free(3); b.printTo(cout);

	b.allocate(); b.printTo(cout);

	return 0;
}
