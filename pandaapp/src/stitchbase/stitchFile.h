// Filename: stitchFile.h
// Created by:  drose (08Nov99)
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

#ifndef STITCHFILE_H
#define STITCHFILE_H

#include "stitchCommand.h"

class StitchImage;
class StitchImageOutputter;

class StitchFile {
public:
  StitchFile();
  ~StitchFile();

  bool read(const string &filename);
  void write(ostream &out) const;

  void process(StitchImageOutputter &outputter);

  StitchCommand _root;
};

inline ostream &operator << (ostream &out, const StitchFile &f) {
  f.write(out);
  return out;
}

#endif


