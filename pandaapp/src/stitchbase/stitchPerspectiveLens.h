// Filename: stitchPerspectiveLens.h
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

#ifndef STITCHPERSPECTIVELENS_H
#define STITCHPERSPECTIVELENS_H

#include "stitchLens.h"

class StitchPerspectiveLens : public StitchLens {
public:
  StitchPerspectiveLens();

  virtual void set_hfov(double fov_deg);

  virtual double get_focal_length(double width_mm) const;
  virtual double get_hfov(double width_mm) const;

  virtual LVector3d extrude(const LPoint2d &point_mm, double width_mm) const;
  virtual LPoint2d project(const LVector3d &vec, double width_mm) const;

  virtual void make_lens_command(StitchCommand *parent);

private:
  double _tan_fov;
};

#endif


