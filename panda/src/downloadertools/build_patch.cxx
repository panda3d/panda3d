// Filename: build_patch.cxx
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

#include "pandabase.h"
#ifndef HAVE_GETOPT
  #include "gnu_getopt.h"
#else
  #include <getopt.h>
#endif
#include "patchfile.h"
#include "filename.h"

int
main(int argc, char *argv[]) {
  if (argc < 3) {
    cerr << "Usage: build_patch <old_file> <new_file>" << endl;
    return 1;
  }

  Filename src_file = Filename::from_os_specific(argv[1]);
  src_file.set_binary();

  Filename dest_file = Filename::from_os_specific(argv[2]);
  dest_file.set_binary();

  Filename patch_file = dest_file.get_fullpath() + ".pch";
  Patchfile pfile;

  cerr << "Building patch file to convert " << src_file << " to "
       << dest_file << endl;
  if (pfile.build(src_file, dest_file, patch_file) == false) {
    cerr << "build patch failed" << endl;
    return 1;
  }

  return 0;
}
