// Filename: test_glob.cxx
// Created by:  drose (30May00)
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

#include "globPattern.h"

int
main(int argc, char *argv[]) {
  if (argc != 2 && argc != 3) {
    cerr
      << "test_glob \"pattern\" [from-directory]\n\n"
      << "Attempts to match the pattern against each of the files in the\n"
      << "indicated directory if specified, or the current directory\n"
      << "otherwise.  Reports all of the matching files.  This is,\n"
      << "of course, exactly the normal behavior of the shell; this test\n"
      << "program merely exercises the Panda GlobPattern object, which\n"
      << "duplicates the shell functionality.\n\n";
    exit(1);
  }

  GlobPattern pattern(argv[1]);
  Filename from_directory;
  if (argc == 3) {
    from_directory = argv[2];
  }

  vector_string results;
  int num_matched = pattern.match_files(results, from_directory);

  cerr << num_matched << " results:\n";
  vector_string::const_iterator si;
  for (si = results.begin(); si != results.end(); ++si) {
    cerr << "  " << *si << "\n";
  }

  return (0);
}


