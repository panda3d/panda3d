// Filename: test_searchpath.cxx
// Created by:  cary (01Sep98)
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

#include "dSearchPath.h"
//#include "expand.h"
#include <string>

void TestSearch()
{
   std::string line, path;

//   path = ".:~ /etc";
   path = ". /etc";
//   path = Expand::Expand(path);
   line = "searchpath.h";
   cout << "looking for file '" << line << "' in path '" << path << "': '";
   line = DSearchPath::search_path(line, path);
   cout << line << "'" << endl;

   line = ".cshrc";
   cout << "looking for file '" << line << "' in path '" << path << "': '";
   line = DSearchPath::search_path(line, path);
   cout << line << "'" << endl;

   line = "passwd";
   cout << "looking for file '" << line << "' in path '" << path << "': '";
   line = DSearchPath::search_path(line, path);
   cout << line << "'" << endl;
}

main()
{
   cout << "Testing search path:" << endl;
   TestSearch();
}
