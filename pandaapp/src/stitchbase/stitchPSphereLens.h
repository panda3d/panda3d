// Filename: stitchPSphereLens.h
// Created by:  drose (16Nov99)
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

#ifndef STITCHPSPHERELENS_H
#define STITCHPSPHERELENS_H

#include "stitchLens.h"

class StitchPSphereLens : public StitchLens {
public:
  StitchPSphereLens();

  virtual double get_focal_length(double width_mm) const;
  virtual double get_hfov(double width_mm) const;

  virtual LVector3d extrude(const LPoint2d &point_mm, double width_mm) const;
  virtual LPoint2d project(const LVector3d &vec, double width_mm) const;

  LPoint2d project_left(const LVector3d &vec, double width_mm) const;
  LPoint2d project_right(const LVector3d &vec, double width_mm) const;

  virtual void draw_triangle(TriangleRasterizer &rast,
                             const LMatrix3d &mm_to_pixels,
                             double width_mm,
                             const RasterizerVertex *v0,
                             const RasterizerVertex *v1,
                             const RasterizerVertex *v2);

  virtual void pick_up_singularity(TriangleRasterizer &rast,
                                   const LMatrix3d &mm_to_pixels,
                                   const LMatrix3d &pixels_to_mm,
                                   const LMatrix3d &rotate,
                                   double width_mm,
                                   StitchImage *input);

  virtual void make_lens_command(StitchCommand *parent);
};

#endif


