// Filename: apply_patch.cxx
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
    cerr << "Usage: apply_patch <patch_file> <old_file>" << endl;
    cerr << "Will overwrite old_file" << endl;
    return 1;
  }

  Filename patch = argv[1];
  patch.set_binary();

  Filename file = argv[2];
  file.set_binary();

  Patchfile pfile;

  cerr << "Applying patch file " << patch << " to " << file << endl;
  if (pfile.apply(patch, file) == false) {
    cerr << "apply patch failed" << endl;
    return 1;
  }

  return 0;
}
