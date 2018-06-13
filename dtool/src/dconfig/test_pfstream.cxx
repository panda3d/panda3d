/**
 * PANDA 3D SOFTWARE
 * Copyright (c) Carnegie Mellon University.  All rights reserved.
 *
 * All use of this software is subject to the terms of the revised BSD
 * license.  You should have received a copy of this license along
 * with this source code in a file named "LICENSE."
 *
 * @file test_pfstream.cxx
 * @author cary
 * @date 1998-08-31
 */

#include "pfstream.h"
#include <string>

void ReadIt(std::istream& ifs) {
   std::string line;

   while (!ifs.eof()) {
      std::getline(ifs, line);
      if (line.length() != 0)
         std::cout << line << std::endl;
   }
}

main()
{
   IPipeStream ipfs("ls -l");

   ReadIt(ipfs);
}
