// Filename: test_expand.cxx
// Created by:  cary (31Aug98)
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

#include "expand.h"
#include <string>

void TestExpandFunction()
{
   std::string line;

   line = "foo";
   cout << "input: '" << line << "'" << endl;
   cout << "output: '" << Expand::Expand(line) << "'" << endl;
   line = "'foo'";
   cout << "input: '" << line << "'" << endl;
   cout << "output: '" << Expand::Expand(line) << "'" << endl;
   line = "'$USER'";
   cout << "input: '" << line << "'" << endl;
   cout << "output: '" << Expand::Expand(line) << "'" << endl;
   line = "$USER";
   cout << "input: '" << line << "'" << endl;
   cout << "output: '" << Expand::Expand(line) << "'" << endl;
   line = "\"$USER\"";
   cout << "input: '" << line << "'" << endl;
   cout << "output: '" << Expand::Expand(line) << "'" << endl;
   line = "`ls -l`";
   cout << "input: '" << line << "'" << endl;
   cout << "output: '" << Expand::Expand(line) << "'" << endl;
   line = "~";
   cout << "input: '" << line << "'" << endl;
   cout << "output: '" << Expand::Expand(line) << "'" << endl;
   line = "~cary";
   cout << "input: '" << line << "'" << endl;
   cout << "output: '" << Expand::Expand(line) << "'" << endl;
}

void TestExpandClass()
{
   std::string line;

   line = "foo";
   Expand::Expander ex(line);
   cout << "input: '" << line << "'" << endl;
   cout << "output: '" << ex() << "'" << endl;
   line = "'foo'";
   cout << "input: '" << line << "'" << endl;
   cout << "output: '" << ex(line) << "'" << endl;
   line = "'$USER'";
   cout << "input: '" << line << "'" << endl;
   cout << "output: '" << ex(line) << "'" << endl;
   line = "$USER";
   cout << "input: '" << line << "'" << endl;
   cout << "output: '" << ex(line) << "'" << endl;
   line = "\"$USER\"";
   cout << "input: '" << line << "'" << endl;
   cout << "output: '" << ex(line) << "'" << endl;
   line = "`ls -l`";
   cout << "input: '" << line << "'" << endl;
   cout << "output: '" << ex(line) << "'" << endl;
   line = "~";
   cout << "input: '" << line << "'" << endl;
   cout << "output: '" << ex(line) << "'" << endl;
   line = "~cary";
   cout << "input: '" << line << "'" << endl;
   cout << "output: '" << ex(line) << "'" << endl;
}

main()
{
   cout << endl << "Testing shell expansion (function version):" << endl;
   TestExpandFunction();
   cout << endl << "Testing shell expansion (class version):" << endl;
   TestExpandClass();
}
