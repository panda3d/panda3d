
#include "stdafx.h"

#include <iostream>
#include <map>
#include <cassert>

using std::cout;
using std::endl;

#include "uniqueIdAllocator.h"


int _tmain(int argc, _TCHAR* argv[]) {
        cout <<"UniqueIdAllocator Test"<<endl;
        UniqueIdAllocator b=UniqueIdAllocator(2, 9);
        b.output(cout, true);
        b.allocate(); b.output(cout, true);
        b.allocate(); b.output(cout, true);
        b.allocate(); b.output(cout, true);
        b.allocate(); b.output(cout, true);
        b.allocate(); b.output(cout, true);
        b.allocate(); b.output(cout, true);
        b.free(2); b.output(cout, true);
        b.free(3); b.output(cout, true);
        b.free(4); b.output(cout, true);
        b.free(5); b.output(cout, true);
        b.allocate(); b.output(cout, true);
        b.allocate(); b.output(cout, true);
        b.allocate(); b.output(cout, true);
        b.free(3); b.output(cout, true);
        b.free(2); b.output(cout, true);

        b.allocate(); b.output(cout, true);
        b.allocate(); b.output(cout, true);
        b.allocate(); b.output(cout, true);
        b.allocate(); b.output(cout, true);
        b.allocate(); b.output(cout, true);
        b.allocate(); b.output(cout, true);
        b.allocate(); b.output(cout, true);

        b.free(4); b.output(cout, true);
        b.free(3); b.output(cout, true);

        b.allocate(); b.output(cout, true);

        return 0;
}
