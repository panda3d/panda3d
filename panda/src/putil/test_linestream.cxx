// Filename: test_linestream.cxx
// Created by:  drose (26Feb00)
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

#include "lineStream.h"

#include "notify.h"

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


