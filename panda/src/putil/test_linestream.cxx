// Filename: test_linestream.cxx
// Created by:  drose (26Feb00)
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

#include "lineStream.h"

#include "pnotify.h"

int
main(int argc, char *argv[]) {
  LineStream ls;

  int i = 0;
  while (ls) {
    if ((i % 5) == 0) {
      if (ls.is_text_available()) {
        nout << "Got line: '" << ls.get_line() << "'";
        if (ls.has_newline()) {
          nout << " (nl)";
        }
        nout << "\n";
      }
    }

    ls << "123456789 ";
    if ((i % 6)==0) {
      ls << "\n";
    }
    i++;
  }

  return 0;
}


