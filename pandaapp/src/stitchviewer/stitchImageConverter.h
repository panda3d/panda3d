// Filename: stitchImageConverter.h
// Created by:  drose (06Nov99)
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

#ifndef STITCHIMAGECONVERTER_H
#define STITCHIMAGECONVERTER_H

#include "stitchImageVisualizer.h"

class StitchImage;

class StitchImageConverter : public StitchImageVisualizer {
public:
  StitchImageConverter();

  virtual void add_output_image(StitchImage *image);

protected:
  virtual void override_chan_cfg(ChanCfgOverrides &override);
  virtual void setup_camera(const RenderRelation &camera_arc);
  virtual bool is_interactive() const;
  virtual void create_image_geometry(Image &im);

  StitchImage *_output_image;
};

#endif
