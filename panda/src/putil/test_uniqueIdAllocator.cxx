/**
 * PANDA 3D SOFTWARE
 * Copyright (c) Carnegie Mellon University.  All rights reserved.
 *
 * All use of this software is subject to the terms of the revised BSD
 * license.  You should have received a copy of this license along
 * with this source code in a file named "LICENSE."
 *
 * @file test_uniqueIdAllocator.cxx
 */

#include "stdafx.h"

#include <iostream>
#include <map>
#include <cassert>
using namespace std;

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
