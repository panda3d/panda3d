// Filename: apply_patch.cxx
// Created by:  
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

#include "pandabase.h"
#include "pystub.h"
#include "panda_getopt.h"
#include "preprocess_argv.h"
#include "patchfile.h"
#include "filename.h"

int
main(int argc, char **argv) {
  // A call to pystub() to force libpystub.so to be linked in.
  pystub();

  preprocess_argv(argc, argv);

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
