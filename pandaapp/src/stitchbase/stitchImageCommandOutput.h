// Filename: stitchImageCommandOutput.h
// Created by:  drose (29Nov99)
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

#ifndef STITCHIMAGECOMMANDOUTPUT_H
#define STITCHIMAGECOMMANDOUTPUT_H

#include "stitchImageOutputter.h"

#include "luse.h"

class Stitcher;
class StitchImage;
class StitchCommand;

class StitchImageCommandOutput : public StitchImageOutputter {
public:
  StitchImageCommandOutput();

  virtual void add_input_image(StitchImage *image);
  virtual void add_output_image(StitchImage *image);
  virtual void add_stitcher(Stitcher *stitcher);

  virtual void execute();

protected:
  void fill_image_cmd(StitchCommand *image_cmd, StitchImage *image);

  typedef vector<StitchImage *> Images;
  Images _input_images;
  Images _output_images;

  typedef vector<Stitcher *> Stitchers;
  Stitchers _stitchers;
};

#endif

