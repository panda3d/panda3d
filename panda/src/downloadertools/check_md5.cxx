// Filename: check_md5.cxx
// Created by:
//
////////////////////////////////////////////////////////////////////
//
// PANDA 3D SOFTWARE
// Copyright (c) 2001, Disney Enterprises, Inc.  All rights reserved
//
// All use of this software is subject to the terms of the Panda 3d
// Software license.  You should have received a copy of this license
// along with this source code; you will also find a current copy of
// the license at http://www.panda3d.org/license.txt .
//
// To contact the maintainers of this program write to
// panda3d@yahoogroups.com .
//
////////////////////////////////////////////////////////////////////

#include "crypto_utils.h"
#include "hashVal.h"
#include "filename.h"

int
main(int argc, char *argv[]) {
  const char *usagestr = "Usage: check_md5 <file>";
  if (argc < 2) {
    cerr << usagestr << endl;
    return 1;
  }

  Filename source_file;
  source_file = Filename::from_os_specific(argv[1]);

  if(!source_file.exists()) {
       cerr << usagestr << endl;
       cerr << source_file << " not found!\n";
       return 2;
  }

  HashVal hash;
  md5_a_file(source_file, hash);

  // output base of Filename along w/md5
  cout << source_file.get_basename() << " ";
  hash.output(cout);
  cout << endl;
  
  return 0;
}
