// Filename: stitchImageProgram.h
// Created by:  drose (16Mar00)
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

#ifndef STITCHIMAGEPROGRAM_H
#define STITCHIMAGEPROGRAM_H

#include <pandatoolbase.h>

#include "stitchCommandReader.h"

////////////////////////////////////////////////////////////////////
//       Class : StitchImageProgram
// Description : A program to read a stitch command file, perform the
//               image manipulations in the CPU, and write output
//               images for each processed image.
////////////////////////////////////////////////////////////////////
class StitchImageProgram : public StitchCommandReader {
public:
  StitchImageProgram();

  void run();

private:
  double _filter_factor;
};

#endif

