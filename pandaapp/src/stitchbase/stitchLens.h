// Filename: stitchLens.h
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

#ifndef STITCHLENS_H
#define STITCHLENS_H

#include "luse.h"

class TriangleRasterizer;
class RasterizerVertex;
class StitchImage;
class StitchCommand;

class StitchLens {
public:
  StitchLens();
  virtual ~StitchLens();

  virtual void set_focal_length(double focal_length_mm);
  virtual void set_hfov(double fov_deg);

  void set_singularity_tolerance(double tol);
  void reset_singularity_detected();

  bool is_defined() const;
  virtual double get_focal_length(double width_mm) const=0;
  virtual double get_hfov(double width_mm) const=0;
  virtual double get_vfov(double height_mm) const;

  virtual LVector3d extrude(const LPoint2d &point_mm, double width_mm) const=0;
  virtual LPoint2d project(const LVector3d &vec, double width_mm) const=0;

  // This function simply passes the indicated triangle on to the
  // rasterizer.  It exists here in the lens so that the lens may do
  // something special if the triangle crosses a seam or singularity
  // in the lens' coordinate space.
  virtual void draw_triangle(TriangleRasterizer &rast,
                             const LMatrix3d &mm_to_pixels,
                             double width_mm,
                             const RasterizerVertex *v0,
                             const RasterizerVertex *v1,
                             const RasterizerVertex *v2);

  // This function is to be called after all triangles have been
  // drawn; it will draw pixel-by-pixel all the points within
  // _singularity_radius of any singularity points the lens may have
  // (these points were not draw by draw_triangle(), above).
  virtual void pick_up_singularity(TriangleRasterizer &rast,
                                   const LMatrix3d &mm_to_pixels,
                                   const LMatrix3d &pixels_to_mm,
                                   const LMatrix3d &rotate,
                                   double width_mm,
                                   StitchImage *input);

  // This generates a StitchCommand that represents the given lens.
  virtual void make_lens_command(StitchCommand *parent)=0;

protected:
  enum Flags {
    F_focal_length = 0x01,
    F_fov          = 0x02,
  };
  int _flags;
  int _singularity_detected;
  double _focal_length;
  double _fov;
  double _singularity_tolerance;
  double _singularity_radius;
};

#endif

