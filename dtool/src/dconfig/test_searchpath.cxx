/**
 * PANDA 3D SOFTWARE
 * Copyright (c) Carnegie Mellon University.  All rights reserved.
 *
 * All use of this software is subject to the terms of the revised BSD
 * license.  You should have received a copy of this license along
 * with this source code in a file named "LICENSE."
 *
 * @file test_searchpath.cxx
 * @author cary
 * @date 1998-09-01
 */

#include "dSearchPath.h"
// #include "expand.h"
#include <string>

using std::cout;
using std::endl;

void TestSearch()
{
   std::string line, path;

// path = ".:~ etc";
   path = ". /etc";
// path = Expand::Expand(path);
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
