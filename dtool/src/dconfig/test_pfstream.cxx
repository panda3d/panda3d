// Filename: test_pfstream.cxx
// Created by:  cary (31Aug98)
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

#include "pfstream.h"
#include <string>

void ReadIt(istream& ifs) {
   std::string line;

   while (!ifs.eof()) {
      std::getline(ifs, line);
      if (line.length() != 0)
         cout << line << endl;
   }
}

main()
{
   IPipeStream ipfs("ls -l");

   ReadIt(ipfs);
}
