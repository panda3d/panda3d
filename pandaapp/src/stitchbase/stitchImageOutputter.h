// Filename: stitchImageOutputter.h
// Created by:  drose (09Nov99)
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

#ifndef STITCHIMAGEOUTPUTTER_H
#define STITCHIMAGEOUTPUTTER_H

class StitchImage;
class Stitcher;

#include <luse.h>

class StitchImageOutputter {
public:
  StitchImageOutputter();
  virtual ~StitchImageOutputter();

  virtual void add_input_image(StitchImage *image)=0;
  virtual void add_output_image(StitchImage *image)=0;
  virtual void add_stitcher(Stitcher *stitcher)=0;

  virtual void execute()=0;
};

#endif
