// Filename: pdecompress.cxx
// Created by:  
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

#include "filename.h"
#include "decompressor.h"

int
main(int argc, char *argv[]) {
  if (argc < 2) {
    cerr << "Usage: pdecompress <file>" << endl;
    return 1;
  }

  Filename source_file = argv[1];
  Decompressor decompressor;
  if (decompressor.decompress(source_file) == false) {
    cerr << "Decompress failed" << endl;
    return 1;
  }

  return 0;
}
