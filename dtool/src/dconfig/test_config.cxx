// Filename: test_config.cxx
// Created by:  cary (10Sep98)
//
////////////////////////////////////////////////////////////////////
//
// PANDA 3D SOFTWARE
// Copyright (c) 2001 - 2004, Disney Enterprises, Inc.  All rights reserved
//
// All use of this software is subject to the terms of the Panda 3d
// Software license.  You should have received a copy of this license
// along with this source code; you will also find a current copy of
// the license at http://etc.cmu.edu/panda3d/docs/license/ .
//
// To contact the maintainers of this program write to
// panda3d-general@lists.sourceforge.net .
//
////////////////////////////////////////////////////////////////////

#include "dconfig.h"

#define SNARF
Configure(test);

std::string foo = test.GetString("user");
std::string path = test.GetString("LD_LIBRARY_PATH");

ConfigureFn(test)
{
   cout << "AIEE!  Doing work before main()!  The sky is falling!" << endl;
}

main()
{
   cout << "Testing Configuration functionality:" << endl;
   cout << "foo = " << foo << endl;
   cout << "path = " << path << endl;
}
