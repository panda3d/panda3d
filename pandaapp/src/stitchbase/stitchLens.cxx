// Filename: stitchLens.cxx
// Created by:  drose (04Nov99)
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

#include "stitchLens.h"
#include "triangleRasterizer.h"

#include "pandatoolbase.h"
#include <deg_2_rad.h>

#include <math.h>

StitchLens::
StitchLens() {
  _flags = 0;
  _singularity_detected = 0;
  set_singularity_tolerance(5.0);
}

StitchLens::
~StitchLens() {
}

void StitchLens::
set_focal_length(double focal_length_mm) {
  _flags |= F_focal_length;
  _focal_length = focal_length_mm;
}

void StitchLens::
set_hfov(double fov_deg) {
  _flags |= F_fov;
  _fov = fov_deg;
}

void StitchLens::
set_singularity_tolerance(double tol) {
  _singularity_tolerance = tol;
  _singularity_radius = sin(deg_2_rad(tol));
}

void StitchLens::
reset_singularity_detected() {
  _singularity_detected = 0;
}

bool StitchLens::
is_defined() const {
  return (_flags != 0);
}

double StitchLens::
get_vfov(double height_mm) const {
  return get_hfov(height_mm);
}

void StitchLens::
draw_triangle(TriangleRasterizer &rast, const LMatrix3d &,
              double, const RasterizerVertex *v0,
              const RasterizerVertex *v1, const RasterizerVertex *v2) {
  rast.draw_triangle(v0, v1, v2);
}

void StitchLens::
pick_up_singularity(TriangleRasterizer &, const LMatrix3d &,
                    const LMatrix3d &, const LMatrix3d &,
                    double, StitchImage *) {
}
