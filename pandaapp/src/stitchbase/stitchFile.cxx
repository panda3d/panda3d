// Filename: stitchFile.cxx
// Created by:  drose (08Nov99)
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

#include "stitchFile.h"
#include "stitchImage.h"
#include "stitchImageOutputter.h"
#include "stitchParserDefs.h"
#include "stitchLexerDefs.h"

StitchFile::
StitchFile() {
}

StitchFile::
~StitchFile() {
}

bool StitchFile::
read(const string &filename) {
  _root.clear();

  ifstream in(filename.c_str());
  if (!in) {
    nout << "Unable to read " << filename << "\n";
    return false;
  }
  stitch_init_parser(in, filename, &_root);
  stitchyyparse();
  if (stitch_error_count() != 0) {
    return false;
  }

  return true;
}

void StitchFile::
write(ostream &out) const {
  _root.write(out, 0);
}

void StitchFile::
process(StitchImageOutputter &outputter) {
  _root.process(outputter, (Stitcher *)NULL, *this);
  outputter.execute();
}
