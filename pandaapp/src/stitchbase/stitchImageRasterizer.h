// Filename: stitchImageRasterizer.h
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

#ifndef STITCHIMAGERASTERIZER_H
#define STITCHIMAGERASTERIZER_H

#include "stitchImageOutputter.h"

#include "pvector.h"
#include "luse.h"
#include "globPattern.h"

class Stitcher;
class StitchImage;

class StitchImageRasterizer : public StitchImageOutputter {
public:
  StitchImageRasterizer();

  virtual void add_input_image(StitchImage *image);
  virtual void add_output_image(StitchImage *image);
  virtual void add_stitcher(Stitcher *stitcher);

  virtual void execute();

  void add_input_name(const string &name);
  bool has_input_name(const string &name) const;
  void add_output_name(const string &name);
  bool has_output_name(const string &name) const;

  void set_filter_factor(double filter_factor);
  void set_output_size(int xsize, int ysize);

private:
  void draw_points(StitchImage *output, StitchImage *input,
                   const Colord &color, double radius);
  void draw_points(StitchImage *output, Stitcher *input,
                   const Colord &color, double radius);
  void draw_image(StitchImage *output, StitchImage *input);

  void draw_spot(StitchImage *output,
                 const LPoint2d pixel_center, const Colord &color,
                 double radius);

  typedef pvector<StitchImage *> Images;
  Images _input_images;
  Images _output_images;

  typedef pvector<Stitcher *> Stitchers;
  Stitchers _stitchers;

  typedef pvector<GlobPattern> Names;
  Names _input_names;
  Names _output_names;

  double _filter_factor;

  int _output_xsize;
  int _output_ysize;
  bool _got_output_size;
};

#endif

