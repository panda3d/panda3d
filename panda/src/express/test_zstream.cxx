// Filename: test_zstream.cxx
// Created by:  drose (05Aug02)
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

#include "pandabase.h"
#include "zStream.h"
#include "filename.h"

void
decompress(istream &source) {
  IDecompressStream zstream(&source, false);

  int ch = zstream.get();
  while (!zstream.eof() && !zstream.fail()) {
    cout.put(ch);
    ch = zstream.get();
  }
}

void
compress(istream &source) {
  OCompressStream zstream(&cout, false);

  int ch = source.get();
  while (!source.eof() && !source.fail()) {
    zstream.put(ch);
    ch = source.get();
  }
}

int
main(int argc, char *argv[]) {
  if (argc != 2) {
    cerr << "test_zstream file\n"
         << "compresses file to standard output, or decompresses it if the\n"
         << "filename ends in .pz.\n";
    return (1);
  }

  Filename source_filename = argv[1];
  source_filename.set_binary();

  ifstream source;

  if (!source_filename.open_read(source)) {
    cerr << "Unable to open source " << source_filename << ".\n";
    return (1);
  }
  if (source_filename.get_extension() == "pz") {
    decompress(source);
  } else {
    compress(source);
  }

  return (0);
}
