// Filename: test_config.cxx
// Created by:  cary (10Sep98)
//
////////////////////////////////////////////////////////////////////
//
// PANDA 3D SOFTWARE
// Copyright (c) Carnegie Mellon University.  All rights reserved.
//
// All use of this software is subject to the terms of the revised BSD
// license.  You should have received a copy of this license along
// with this source code in a file named "LICENSE."
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
