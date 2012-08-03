// Filename: build_patch.cxx
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

void 
usage() {
  cerr << "Usage: build_patch [opts] <old_file> <new_file>" << endl;
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
    "to be much faster.  The -c option will forbid this and force the patcher\n"
    "to work on the full file.\n\n"

    "Options:\n\n"

    "    -o output_name\n"
    "        Specify the filename of the patch file to generate.\n\n"

    "    -c\n"
    "        Always generate patches against the complete file, even if the\n"
    "        input files appear to be multifiles.\n\n"

    "    -f footprint_length\n"
    "        Specify the footprint length for the patching algorithm.\n\n";
}

int
main(int argc, char **argv) {
  // A call to pystub() to force libpystub.so to be linked in.
  pystub();

  Filename patch_file;
  bool complete_file = false;
  int footprint_length = 0;

  //  extern char *optarg;
  extern int optind;
  static const char *optflags = "o:cf:h";
  preprocess_argv(argc, argv);
  int flag = getopt(argc, argv, optflags);
  Filename rel_path;
  while (flag != EOF) {
    switch (flag) {
    case 'o':
      patch_file = optarg;
      break;

    case 'c':
      complete_file = true;
      break;

    case 'f':
      footprint_length = atoi(optarg);
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

  if (patch_file.empty()) {
    patch_file = dest_file.get_fullpath() + ".pch";
  }
  Patchfile pfile;

  pfile.set_allow_multifile(!complete_file);
  if (footprint_length != 0) {
    cerr << "Footprint length is " << footprint_length << "\n";
    pfile.set_footprint_length(footprint_length);
  }

  cerr << "Building patch file to convert " << src_file << " to "
       << dest_file << endl;
  if (pfile.build(src_file, dest_file, patch_file) == false) {
    cerr << "build patch failed" << endl;
    return 1;
  }

  return 0;
}
