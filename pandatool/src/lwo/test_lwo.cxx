// Filename: test_lwo.cxx
// Created by:  drose (24Apr01)
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

#include "lwoInputFile.h"
#include "lwoChunk.h"
#include "config_lwo.h"
#include "pystub.h"

int
main(int argc, char *argv[]) {
  // A call to pystub() to force libpystub.so to be linked in.
  pystub();

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
