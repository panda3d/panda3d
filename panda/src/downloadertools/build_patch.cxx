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

void 
usage() {
  cerr << "Usage: build_patch [-f] <old_file> <new_file>" << endl;
}

void
help() {
  usage();
  cerr << "\n"
    "This program generates a patch file that describes the differences\n"
    "between any two source files.  The patch file can later be used to\n"
    "construct <new_file>, given <old_file>.  Arbitrary file types, including\n"
    "binary files, are supported.\n\n"

    "The patching algorithm can get very slow for very large files.  As an\n"
    "optimization, if the input files are both Panda Multifiles, the patcher\n"
    "will by default patch them on a per-subfile basis, which has the potential\n"
    "to be much faster.  The -f option will forbid this and force the patcher\n"
    "to work on the full file.\n\n";
}

int
main(int argc, char *argv[]) {
  bool full_file = false;

  //  extern char *optarg;
  extern int optind;
  static const char *optflags = "fh";
  int flag = getopt(argc, argv, optflags);
  Filename rel_path;
  while (flag != EOF) {
    switch (flag) {
    case 'f':
      full_file = true;
      break;

    case 'h':
      help();
      return 1;
    case '?':
      usage();
      return 1;
    default:
      cerr << "Unhandled switch: " << flag << endl;
      break;
    }
    flag = getopt(argc, argv, optflags);
  }
  argc -= (optind - 1);
  argv += (optind - 1);

  if (argc < 3) {
    usage();
    return 1;
  }

  Filename src_file = Filename::from_os_specific(argv[1]);
  src_file.set_binary();

  Filename dest_file = Filename::from_os_specific(argv[2]);
  dest_file.set_binary();

  Filename patch_file = dest_file.get_fullpath() + ".pch";
  Patchfile pfile;

  pfile.set_allow_multifile(!full_file);

  cerr << "Building patch file to convert " << src_file << " to "
       << dest_file << endl;
  if (pfile.build(src_file, dest_file, patch_file) == false) {
    cerr << "build patch failed" << endl;
    return 1;
  }

  return 0;
}
