// Filename: test_lwo.cxx
// Created by:  drose (24Apr01)
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

#include "lwoInputFile.h"
#include "lwoChunk.h"
#include "config_lwo.h"

int
main(int argc, char *argv[]) {
  init_liblwo();
  if (argc != 2) {
    nout << "test_lwo file.lwo\n";
    exit(1);
  }

  LwoInputFile in;
  if (!in.open_read(argv[1])) {
    nout << "Unable to open " << argv[1] << "\n";
    exit(1);
  }

  PT(IffChunk) chunk = in.get_chunk();
  while (chunk != (IffChunk *)NULL) {
    nout << "Got chunk type " << chunk->get_type() << ":\n";
    chunk->write(nout, 2);
    chunk = in.get_chunk();
  }

  nout << "EOF = " << in.is_eof() << "\n";

  return (0);
}
