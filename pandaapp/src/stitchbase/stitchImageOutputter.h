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

#include "pandaappbase.h"

#include "stitchMultiScreen.h"
#include "luse.h"
#include "pointerTo.h"

class StitchImage;
class Stitcher;

////////////////////////////////////////////////////////////////////
//       Class : StitchImageOutputter
// Description : This is an abstract base class defining the interface
//               to accept a number of input images and process them,
//               possibly stitching them together based on their
//               points in common, and then generating a number of
//               output images.
//
//               This is the highest level of interaction with this
//               module; it corresponds to the action performed by a
//               stitch-image, stitch-viewer, or stitch-command
//               program (and the distinction between the three
//               programs is primarily a question of which
//               specialization of StitchImageOutputter they use).
////////////////////////////////////////////////////////////////////
class StitchImageOutputter {
public:
  StitchImageOutputter();
  virtual ~StitchImageOutputter();

  virtual void add_input_image(StitchImage *image)=0;
  virtual void add_output_image(StitchImage *image)=0;
  virtual void add_stitcher(Stitcher *stitcher)=0;
  void add_screen(StitchScreen *screen);

  virtual void set_eyepoint(const LMatrix4d &mat);

  virtual void execute()=0;

protected:
  PT(StitchMultiScreen) _screen;
};

#endif
